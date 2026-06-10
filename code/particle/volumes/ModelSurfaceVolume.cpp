#include "ModelSurfaceVolume.h"

#include "math/vecmat.h"

namespace particle {
ModelSurfaceVolume::ModelSurfaceVolume() : m_modelScale(::util::UniformFloatRange(1.f)), m_modular_curve_instance(m_modular_curves.create_instance()) {
	Model_load_clear_CPU_buffers = false;
};

vec3d ModelSurfaceVolume::sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction, const EffectHost& host) {
	auto attachment = host.getParentAttachment();
	int obj_num = -1;
	if (auto* obj = std::get_if<effects::attachment_object>(&attachment))
		obj_num = obj->objnum;
	int submodel = host.getParentSubmodel();

	vec3d point = ZERO_VECTOR;

	auto curveSource = std::tuple_cat(source, std::make_tuple(particlesFraction));

	if (obj_num >= 0) {
		const polymodel* pm = object_get_model(&Objects[obj_num]);
		if (pm != nullptr) {
			if (submodel < 0) {
				SCP_vector<int> eligible_submodels;
				for (int i = 0; i < pm->n_models; ++i) {
					if (!pm->submodel[i].flags[Model::Submodel_flags::Is_lod, Model::Submodel_flags::Is_damaged, Model::Submodel_flags::Is_live_debris])
						eligible_submodels.emplace_back(i);
				}

				if (!eligible_submodels.empty())
					submodel = eligible_submodels[::util::UniformUIntRange(0U, static_cast<unsigned int>(eligible_submodels.size()) - 1).next()];
			}

			if (submodel >= 0) {
				const bsp_info* submodel_data = &pm->submodel[submodel];
				const auto* geometry_data = submodel_data->buffer.model_list;
				Assertion(geometry_data != nullptr, "ModelSurfaceVolume particles were spawned on a model with no data available!");

				size_t target_vertex = ::util::UniformUIntRange(0U, static_cast<unsigned int>(geometry_data->n_verts) - 1).next();

				//This point is, despite its name, not in world space, but in model local space (NOT submodel local though!)
				point = geometry_data->vert[target_vertex].world;

				point *= m_modelScale.next() * m_modular_curves.get_output(VolumeModularCurveOutput::SCALE_MULT, curveSource, &m_modular_curve_instance);
			}
		}
	}

	return pointCompensateForOffsetAndRotOffset(point, orientation,
				m_modular_curves.get_output(VolumeModularCurveOutput::OFFSET_ROT, curveSource, &m_modular_curve_instance),
				m_modular_curves.get_output(VolumeModularCurveOutput::POINT_TO_ROT, curveSource, &m_modular_curve_instance));
}

void ModelSurfaceVolume::parse() {
	if (optional_string("+Scale:")) {
		m_modelScale = ::util::ParsedRandomFloatRange::parseRandomRange();
	}

	ParticleVolume::parseCommon();

	m_modular_curves.parse("$Volume Curve:");
}
}
