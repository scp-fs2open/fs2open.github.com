/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "gropenglshader.h"
#include "graphics/material.h"
#include "gropenglstate.h"
#include "math/vecmat.h"

extern GLfloat GL_max_anisotropy;


opengl_state GL_state;


opengl_texture_state::~opengl_texture_state()
{
	if (units != NULL) {
		vm_free(units);
	}
}

void opengl_texture_state::init(GLuint n_units)
{
	Assert( n_units > 0 );
	units = (opengl_texture_unit*) vm_malloc(n_units * sizeof(opengl_texture_unit));
	num_texture_units = n_units;

	for (unsigned int unit = 0; unit < num_texture_units; unit++) {
		units[unit].enabled = GL_FALSE;

		default_values(unit);

		glActiveTexture(GL_TEXTURE0 + unit);
	}

	SetActiveUnit();
}

void opengl_texture_state::default_values(GLint unit, GLenum target)
{
	glActiveTexture(GL_TEXTURE0 + unit);

	if (target == GL_INVALID_ENUM) {

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		units[unit].texture_target = GL_TEXTURE_2D;
		units[unit].texture_id = 0;
	}
}

void opengl_texture_state::SetTarget(GLenum tex_target)
{
	if (units[active_texture_unit].texture_target != tex_target) {

		if (units[active_texture_unit].texture_id) {
			glBindTexture(units[active_texture_unit].texture_target, 0);
			units[active_texture_unit].texture_id = 0;
		}

		// reset modes, since those were only valid for the previous texture target
		default_values(active_texture_unit, tex_target);
		units[active_texture_unit].texture_target = tex_target;
	}
}

void opengl_texture_state::SetActiveUnit(GLuint id)
{
	if (id >= num_texture_units) {
		Int3();
		id = 0;
	}

	glActiveTexture(GL_TEXTURE0 + id);

	active_texture_unit = id;
}

void opengl_texture_state::Enable(GLuint tex_id)
{
	if ( units[active_texture_unit].texture_id == tex_id ) {
		return;
	}

	if (units[active_texture_unit].texture_id != tex_id) {
		glBindTexture(units[active_texture_unit].texture_target, tex_id);
		units[active_texture_unit].texture_id = tex_id;
	}
}

void opengl_texture_state::Enable(GLuint unit, GLenum tex_target, GLuint tex_id) {
	Assertion(unit < num_texture_units, "Invalid texture unit value!");

	if (units[unit].texture_target == tex_target && units[unit].texture_id == tex_id) {
		// The texture unit already uses this texture. There is no need to change it
		return;
	}

	// Go the standard route
	SetActiveUnit(unit);
	SetTarget(tex_target);
	Enable(tex_id);
}

void opengl_texture_state::Delete(GLuint tex_id)
{
	if (tex_id == 0) {
		Int3();
		return;
	}

	GLuint atu_save = active_texture_unit;

	for (unsigned int i = 0; i < num_texture_units; i++) {
		if (units[i].texture_id == tex_id) {
			SetActiveUnit(i);

			glBindTexture(units[i].texture_target, 0);
			units[i].texture_id = 0;

			default_values(i, units[i].texture_target);

			if (i == atu_save) {
				atu_save = 0;
			}
		}
	}

	SetActiveUnit(atu_save);
}

void opengl_state::init()
{
	int i;
	
	glDisable(GL_BLEND);
	blend_Status = GL_FALSE;

	glDisable(GL_DEPTH_TEST);
	depthtest_Status = GL_FALSE;

	glDisable(GL_SCISSOR_TEST);
	scissortest_Status = GL_FALSE;

	glDisable(GL_CULL_FACE);
	cullface_Status = GL_FALSE;

	glDisable(GL_POLYGON_OFFSET_FILL);
	polygonoffsetfill_Status = GL_FALSE;

	polygon_offset_Factor = 0.0f;
	polygon_offset_Unit = 0.0f;

	normalize_Status = GL_FALSE;

	for (i = 0; i < (int)(sizeof(clipplane_Status) / sizeof(GLboolean)); i++) {
		//glDisable(GL_CLIP_PLANE0+i);
		clipplane_Status[i] = GL_FALSE;
	}

	for (i = 0; i < (int)(sizeof(clipdistance_Status) / sizeof(GLboolean)); i++) {
		//glDisable(GL_CLIP_DISTANCE0+i);
		clipdistance_Status[i] = GL_FALSE;
	}

	glDepthMask(GL_FALSE);
	depthmask_Status = GL_FALSE;

	glFrontFace(GL_CCW);
	frontface_Value = GL_CCW;

	glCullFace(GL_BACK);
	cullface_Value = GL_BACK;

	glBlendFunc(GL_ONE, GL_ZERO);
	blendfunc_Value[0] = GL_ONE;
	blendfunc_Value[1] = GL_ZERO;

	glDepthFunc(GL_LESS);
	depthfunc_Value = GL_LESS;

	glGetFloatv(GL_LINE_WIDTH, &line_width_Value);

	Current_alpha_blend_mode = ALPHA_BLEND_NONE;

	current_program = 0;
	glUseProgram(0);

	current_framebuffer = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	framebuffer_stack.clear();
}

GLboolean opengl_state::Blend(GLint state)
{
	GLboolean save_state = blend_Status;

	if ( !((state == -1) || (state == blend_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_BLEND);
			blend_Status = GL_TRUE;
		} else {
			glDisable(GL_BLEND);
			blend_Status = GL_FALSE;
		}

		Current_alpha_blend_mode = (gr_alpha_blend)(-1);
	}

	return save_state;
}

GLboolean opengl_state::DepthTest(GLint state)
{
	GLboolean save_state = depthtest_Status;

	if ( !((state == -1) || (state == depthtest_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_DEPTH_TEST);
			depthtest_Status = GL_TRUE;
		} else {
			glDisable(GL_DEPTH_TEST);
			depthtest_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::ScissorTest(GLint state)
{
	GLboolean save_state = scissortest_Status;

	if ( !((state == -1) || (state == scissortest_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_SCISSOR_TEST);
			scissortest_Status = GL_TRUE;
		} else {
			glDisable(GL_SCISSOR_TEST);
			scissortest_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::StencilTest(GLint state)
{
    GLboolean save_state = stenciltest_Status;

    if ( !((state == -1) || (state == stenciltest_Status)) ) {
        if (state) {
            Assert( state == GL_TRUE );
            glEnable(GL_STENCIL_TEST);
            stenciltest_Status = GL_TRUE;
        } else {
            glDisable(GL_STENCIL_TEST);
            stenciltest_Status = GL_FALSE;
        }
    }

    return save_state;
}

GLboolean opengl_state::CullFace(GLint state)
{
	GLboolean save_state = cullface_Status;

	if ( !((state == -1) || (state == cullface_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_CULL_FACE);
			cullface_Status = GL_TRUE;
		} else {
			glDisable(GL_CULL_FACE);
			cullface_Status = GL_FALSE;
		}
	}

	return save_state;
}

void opengl_state::SetPolygonMode(GLenum face, GLenum mode)
{
	if ( polygon_mode_Face != face || polygon_mode_Mode != mode ) {
		glPolygonMode(face, mode);

		polygon_mode_Face = face;
		polygon_mode_Mode = mode;
	}
}

void opengl_state::SetPolygonOffset(GLfloat factor, GLfloat units)
{
	if ( polygon_offset_Factor != factor || polygon_offset_Unit != units) {
		glPolygonOffset(factor, units);

		polygon_offset_Factor = factor;
		polygon_offset_Unit = units;
	}
}

GLboolean opengl_state::PolygonOffsetFill(GLint state)
{
	GLboolean save_state = polygonoffsetfill_Status;

	if ( !((state == -1) || (state == polygonoffsetfill_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glEnable(GL_POLYGON_OFFSET_FILL);
			polygonoffsetfill_Status = GL_TRUE;
		} else {
			glDisable(GL_POLYGON_OFFSET_FILL);
			polygonoffsetfill_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::ClipDistance(GLint num, bool state)
{
	Assert( (num >= 0) && (num < (int)(sizeof(clipdistance_Status) / sizeof(GLboolean))) );

	GLboolean save_state = clipdistance_Status[num];

	if (state != clipdistance_Status[num]) {
		if (state) {
			glEnable(GL_CLIP_DISTANCE0+num);
		} else {
			glDisable(GL_CLIP_DISTANCE0+num);
		}
		clipdistance_Status[num] = state;
	}

	return save_state;
}

GLboolean opengl_state::DepthMask(GLint state)
{
	GLboolean save_state = depthmask_Status;

	if ( !((state == -1) || (state == depthmask_Status)) ) {
		if (state) {
			Assert( state == GL_TRUE );
			glDepthMask(GL_TRUE);
			depthmask_Status = GL_TRUE;
		} else {
			glDepthMask(GL_FALSE);
			depthmask_Status = GL_FALSE;
		}
	}

	return save_state;
}

GLboolean opengl_state::ColorMask(GLint state)
{
    GLboolean save_state = colormask_Status;

    if ( !((state == -1) || (state == colormask_Status)) ) {
        if (state) {
            Assert( state == GL_TRUE );
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            colormask_Status = GL_TRUE;
        } else {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            colormask_Status = GL_FALSE;
        }
    }

    return save_state;
}

void opengl_state::SetAlphaBlendMode(gr_alpha_blend ab)
{
	if (ab == Current_alpha_blend_mode) {
		return;
	}

	switch (ab) {
		case ALPHA_BLEND_ALPHA_BLEND_ALPHA:
			GL_state.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case ALPHA_BLEND_NONE:
			GL_state.BlendFunc(GL_ONE, GL_ZERO);
			break;

		case ALPHA_BLEND_ADDITIVE:
			GL_state.BlendFunc(GL_ONE, GL_ONE);
			break;

		case ALPHA_BLEND_ALPHA_ADDITIVE:
			GL_state.BlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;

		case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:
			GL_state.BlendFunc(/*GL_SRC_COLOR*/GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
			break;

		case ALPHA_BLEND_PREMULTIPLIED:
			GL_state.BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;

		default:
			break;
	}

	GL_state.Blend( (ab == ALPHA_BLEND_NONE) ? GL_FALSE : GL_TRUE );

	Current_alpha_blend_mode = ab;
}

void opengl_state::SetZbufferType(gr_zbuffer_type zt)
{
	switch (zt) {
		case ZBUFFER_TYPE_NONE:
			GL_state.DepthFunc(GL_ALWAYS);
			GL_state.DepthMask(GL_FALSE);
			break;

		case ZBUFFER_TYPE_READ:
			GL_state.DepthFunc(GL_LESS);
			GL_state.DepthMask(GL_FALSE);
			break;

		case ZBUFFER_TYPE_WRITE:
			GL_state.DepthFunc(GL_ALWAYS);
			GL_state.DepthMask(GL_TRUE);
			break;

		case ZBUFFER_TYPE_FULL:
			GL_state.DepthFunc(GL_LESS);
			GL_state.DepthMask(GL_TRUE);
			break;

		default:
			break;
	}

	GL_state.DepthTest( (zt == ZBUFFER_TYPE_NONE) ? GL_FALSE : GL_TRUE );
}

void opengl_state::SetStencilType(gr_stencil_type st)
{
    if (st == Current_stencil_type) {
        return;
    }
    
    switch (st) {
        case STENCIL_TYPE_NONE:
            glStencilFunc( GL_NEVER, 1, 0xFFFF );
            glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
            break;
            
        case STENCIL_TYPE_READ:
            glStencilFunc( GL_NOTEQUAL, 1, 0XFFFF );
            glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
            break;
            
        case STENCIL_TYPE_WRITE:
            glStencilFunc( GL_ALWAYS, 1, 0xFFFF );
            glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
            break;
                     
        default:
            break;
    }
           
    GL_state.StencilTest( (st == STENCIL_TYPE_NONE) ? GL_FALSE : GL_TRUE );
         
    Current_stencil_type = st;
}

void opengl_state::SetLineWidth(GLfloat width)
{
	if ( width == line_width_Value ) {
		return;
	}

	glLineWidth(width);
	line_width_Value = width;
}
void opengl_state::UseProgram(GLuint program)
{
	if (current_program == program) {
		return;
	}

	current_program = program;
	glUseProgram(program);
}
bool opengl_state::IsCurrentProgram(GLuint program) {
	return current_program == program;
}
void opengl_state::BindFrameBuffer(GLuint name) {
	if (current_framebuffer != name) {
		glBindFramebuffer(GL_FRAMEBUFFER, name);
		current_framebuffer = name;
	}
}
void opengl_state::PushFramebufferState() {
	framebuffer_stack.push_back(current_framebuffer);
}
void opengl_state::PopFramebufferState() {
	Assertion(framebuffer_stack.size() > 0, "Tried to pop the framebuffer state stack while it was empty!");

	auto restoreBuffer = framebuffer_stack.back();
	framebuffer_stack.pop_back();

	BindFrameBuffer(restoreBuffer);
}
void opengl_state::BindVertexArray(GLuint vao) {
	if (current_vao == vao) {
		return;
	}

	glBindVertexArray(vao);
	current_vao = vao;

	Array.VertexArrayChanged();
}

opengl_array_state::~opengl_array_state()
{
	if ( client_texture_units != NULL ) {
		vm_free(client_texture_units);
	}
}

void opengl_array_state::init(GLuint n_units)
{
	Assert( n_units > 0 );
	client_texture_units = (opengl_client_texture_unit*) vm_malloc(n_units * sizeof(opengl_client_texture_unit));
	num_client_texture_units = n_units;
	active_client_texture_unit = 0;

	for (unsigned int i = 0; i < num_client_texture_units; i++) {
		client_texture_units[i].pointer = 0;
		client_texture_units[i].size = 4;
		client_texture_units[i].status = GL_FALSE;
		client_texture_units[i].stride = 0;
		client_texture_units[i].type = GL_FLOAT;
		client_texture_units[i].buffer = 0;
		client_texture_units[i].reset_ptr = false;
		client_texture_units[i].used_for_draw = false;
	}

	array_buffer = 0;
	element_array_buffer = 0;
	texture_array_buffer = 0;
	uniform_buffer = 0;
}

void opengl_array_state::EnableVertexAttrib(GLuint index)
{
	opengl_vertex_attrib_unit *va_unit = &vertex_attrib_units[index];

	va_unit->used_for_draw = true;

	if ( va_unit->status_init && va_unit->status == GL_TRUE ) {
		return;
	}

	glEnableVertexAttribArray(index);
	va_unit->status = GL_TRUE;
	va_unit->status_init = true;
}

void opengl_array_state::DisableVertexAttrib(GLuint index)
{
	opengl_vertex_attrib_unit *va_unit = &vertex_attrib_units[index];

	if ( va_unit->status_init && va_unit->status == GL_FALSE ) {
		return;
	}

	glDisableVertexAttribArray(index);
	va_unit->status = GL_FALSE;
	va_unit->status_init = true;
}

void opengl_array_state::VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer)
{
	opengl_vertex_attrib_unit *va_unit = &vertex_attrib_units[index];

	if ( 
		!va_unit->reset_ptr 
		&& va_unit->ptr_init 
		&& va_unit->normalized == normalized 
		&& va_unit->pointer == pointer 
		&& va_unit->size == size 
		&& va_unit->stride == stride 
		&& va_unit->type == type 
		&& va_unit->buffer == array_buffer
	) {
		return;
	}

	glVertexAttribPointer(index, size, type, normalized, stride, pointer);

	va_unit->normalized = normalized;
	va_unit->pointer = pointer;
	va_unit->size = size;
	va_unit->stride = stride;
	va_unit->type = type;
	va_unit->buffer = array_buffer;
	va_unit->reset_ptr = false;

	va_unit->ptr_init = true;
}

void opengl_array_state::ResetVertexAttribs()
{
	SCP_map<GLuint, opengl_vertex_attrib_unit>::iterator it;

	for ( it = vertex_attrib_units.begin(); it != vertex_attrib_units.end(); ++it ) {
		DisableVertexAttrib(it->first);
	}

	vertex_attrib_units.clear();
}

void opengl_array_state::BindPointersBegin()
{
	for (unsigned int i = 0; i < num_client_texture_units; i++) {
		client_texture_units[i].used_for_draw = false;
	}

	SCP_map<GLuint, opengl_vertex_attrib_unit>::iterator it;

	for (it = vertex_attrib_units.begin(); it != vertex_attrib_units.end(); ++it) {
		it->second.used_for_draw = false;
	}
}

void opengl_array_state::BindPointersEnd()
{
	SCP_map<GLuint, opengl_vertex_attrib_unit>::iterator it;

	for (it = vertex_attrib_units.begin(); it != vertex_attrib_units.end(); ++it) {
		if ( !it->second.used_for_draw ) DisableVertexAttrib(it->first);
	}
}

void opengl_array_state::BindArrayBuffer(GLuint id)
{
	if ( array_buffer_valid && array_buffer == id ) {
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, id);

	array_buffer = id;
	array_buffer_valid = true;

	for (unsigned int i = 0; i < num_client_texture_units; i++) {
		client_texture_units[i].reset_ptr = true;
	}

	SCP_map<GLuint,opengl_vertex_attrib_unit>::iterator it;

	for ( it = vertex_attrib_units.begin(); it != vertex_attrib_units.end(); ++it ) {
		it->second.reset_ptr = true;
	}
}

void opengl_array_state::BindElementBuffer(GLuint id)
{
	if ( element_array_buffer_valid && element_array_buffer == id ) {
		return;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);

	element_array_buffer = id;
	element_array_buffer_valid = true;
}

void opengl_array_state::BindTextureBuffer(GLuint id)
{
	if ( texture_array_buffer == id ) {
		return;
	}

	glBindBuffer(GL_TEXTURE_BUFFER, id);

	texture_array_buffer = id;
}

void opengl_array_state::BindUniformBufferBindingIndex(GLuint id, GLuint index)
{
	if ( uniform_buffer_index_bindings[index] == id ) {
		return;
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, index, id);

	uniform_buffer_index_bindings[index] = id;
}

void opengl_array_state::BindUniformBuffer(GLuint id)
{
	if ( uniform_buffer == id ) {
		return;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, id);

	uniform_buffer = id;
}
void opengl_array_state::VertexArrayChanged() {
	array_buffer_valid = false;
	element_array_buffer_valid = false;

	for (auto& bindingInfo : vertex_buffer_bindings) {
		bindingInfo.valid_data = false;
	}
}
void opengl_array_state::BindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {
	if (bindingindex >= vertex_buffer_bindings.size()) {
		// Make sure that we have the place for this information
		vertex_buffer_bindings.resize(bindingindex + 1);
	}

	auto& bindingInfo = vertex_buffer_bindings[bindingindex];

	if (bindingInfo.valid_data && bindingInfo.buffer == buffer && bindingInfo.offset == offset
		&& bindingInfo.stride == stride) {
		return;
	}

	glBindVertexBuffer(bindingindex, buffer, offset, stride);

	bindingInfo.valid_data = true;
	bindingInfo.buffer = buffer;
	bindingInfo.stride = stride;
	bindingInfo.offset = offset;
}
opengl_constant_state::opengl_constant_state() {
}
void opengl_constant_state::init() {
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_uniform_buffer_offset_alignment);
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &_max_uniform_block_size);
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &_max_uniform_block_bindings);
}
GLint opengl_constant_state::GetUniformBufferOffsetAlignment() {
	return _uniform_buffer_offset_alignment;
}
GLint opengl_constant_state::GetMaxUniformBlockSize() {
	return _max_uniform_block_size;
}
GLint opengl_constant_state::GetMaxUniformBlockBindings() {
	return _max_uniform_block_bindings;
}

void gr_opengl_clear_states()
{
	gr_zbias(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_cull(0);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	opengl_shader_set_current();
}
