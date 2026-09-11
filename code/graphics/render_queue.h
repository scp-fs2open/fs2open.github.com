#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/util/UniformBuffer.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"

class model_batch_buffer
{
	SCP_vector<matrix4> Submodel_matrices;
	void* Mem_alloc;
	size_t Mem_alloc_size;

	size_t Current_offset;

	void allocate_memory();
public:
	model_batch_buffer() : Mem_alloc(nullptr), Mem_alloc_size(0), Current_offset(0) {};

	void reset();

	size_t get_buffer_offset() const;
	void set_num_models(int n_models);
	void set_model_transform(const matrix4 &transform, int model_id);

	void submit_buffer_data();

	void add_matrix(const matrix4 &mat);
};

template <typename Derived, typename DrawEntryT>
class render_queue {
protected:
	SCP_vector<DrawEntryT> _elements;
	SCP_vector<int> _keys;
	transform_stack _transforms;
	scene_lights _lights;
	graphics::util::UniformBuffer _dataBuffer;
	vec3d _scale;
	inline static model_batch_buffer _batchBuffer; // shared across instances so we don't need to do the malloc again
	bool _initialized = false;

public:
	render_queue()
	{
		_scale.xyz.x = 1.0f;
		_scale.xyz.y = 1.0f;
		_scale.xyz.z = 1.0f;
	}

	void reset()
	{
		_elements.clear();
		_keys.clear();
		_transforms.clear();
		_scale.xyz.x = 1.0f;
		_scale.xyz.y = 1.0f;
		_scale.xyz.z = 1.0f;
		_initialized = false;
		_batchBuffer.reset();
	}

	void push_transform(const vec3d* pos, const matrix* orient)
	{
		_transforms.push(pos, orient);
	}

	void pop_transform()
	{
		_transforms.pop();
	}

	const matrix4& get_transform() const
	{
		return _transforms.get_transform();
	}

	void clear_transforms()
	{
		_transforms.clear();
	}

	void set_scale(const vec3d* s)
	{
		if (s == nullptr) {
			_scale.xyz.x = 1.0f;
			_scale.xyz.y = 1.0f;
			_scale.xyz.z = 1.0f;
		} else {
			_scale = *s;
		}
	}

	void start_model_batch(int n_models)
	{
		_batchBuffer.set_num_models(n_models);
	}

	void add_submodel_to_batch(int model_num)
	{
		matrix4 transform = _transforms.get_transform();

		vm_vec_scale(&transform.vec.rvec, _scale.xyz.x);
		vm_vec_scale(&transform.vec.uvec, _scale.xyz.y);
		vm_vec_scale(&transform.vec.fvec, _scale.xyz.z);

		transform.a1d[15] = 0.0f;

		_batchBuffer.set_model_transform(transform, model_num);
	}

	void init_render(bool sort = true)
	{
		if (sort) {
			self().sort_draws();
		}

		_batchBuffer.submit_buffer_data();

		self().build_uniform_buffer();

		_initialized = true;
	}

	void render_all()
	{
		Assertion(_initialized, "init_render must be called before any render_all call!");

		_lights.resetLightState();

		for (const int& _key : _keys) {
			self().render_buffer(_elements[_key]);
		}

		gr_alpha_mask_set(0, 1.0f);
	}

	void push_element(DrawEntryT&& entry)
	{
		_elements.push_back(std::move(entry));
		_keys.push_back(static_cast<int>(_elements.size() - 1));
	}

	void sort_draws()
	{
		const auto& d = self();
		std::sort(_keys.begin(), _keys.end(),
		          [&d](int a, int b) { return d.sort_draw_pair(a, b); });
	}

private:
	Derived& self() { return static_cast<Derived&>(*this); }
	const Derived& self() const { return static_cast<const Derived&>(*this); }
};
