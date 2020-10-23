#include "lab/dialogs/material_overrides.h"


void labviewer_set_material_override_diffuse_red(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::DiffuseRed, value);
}

void labviewer_set_material_override_diffuse_green(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::DiffuseGreen, value);
}

void labviewer_set_material_override_diffuse_blue(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::DiffuseBlue, value);
}

void labviewer_set_material_override_glow_red(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::GlowRed, value);
}

void labviewer_set_material_override_glow_green(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::GlowGreen, value);
}

void labviewer_set_material_override_glow_blue(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::GlowBlue, value);
}

void labviewer_set_material_override_specular_red(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::SpecularRed, value);
}

void labviewer_set_material_override_specular_green(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::SpecularGreen, value);
}

void labviewer_set_material_override_specular_blue(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::SpecularBlue, value);
}

void labviewer_set_material_override_specular_gloss(Slider* caller) {
	auto value = caller->GetSliderValue() / 255.0f;

	getLabManager()->Renderer->setTextureChannelValue(TextureChannel::SpecularGloss, value);
}

void set_diffuse_override(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setTextureOverride(TextureOverride::Diffuse, value);
}

void set_glow_override(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setTextureOverride(TextureOverride::Glow, value);
}

void set_spec_override(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setTextureOverride(TextureOverride::Specular, value);
}

void MaterialOverrides::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Material Overrides", gr_screen.max_w - 300, 200));
	dialogWindow->SetOwner(this);

	dialogWindow->DeleteChildren();

	auto y = 0;
	auto cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Override Diffuse Color", 2, y, set_diffuse_override));
	
	y += cbp->GetHeight() + 1;

	auto sldr = (Slider*)dialogWindow->AddChild(
		new Slider("Red", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_diffuse_red, 275));
	y += sldr->GetHeight() + 1;

	sldr = (Slider*)dialogWindow->AddChild(
		new Slider("Green", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_diffuse_green, 275));
	y += sldr->GetHeight() + 1;

	sldr = (Slider*)dialogWindow->AddChild(
		new Slider("Blue", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_diffuse_blue, 275));
	y += sldr->GetHeight() + 1;

	if (Cmdline_glow) {
		cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Override Glow Color", 2, y, set_glow_override));
		y += cbp->GetHeight() + 1;

		sldr = (Slider*)dialogWindow->AddChild(
			new Slider("Red", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_glow_red, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)dialogWindow->AddChild(
			new Slider("Green", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_glow_green, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)dialogWindow->AddChild(
			new Slider("Blue", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_glow_blue, 275));
		y += sldr->GetHeight() + 1;
	}
	if (Cmdline_spec) {
		cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Override Spec Color", 2, y, set_spec_override));
		cbp->SetBool(&Specmap_color_override_set);
		y += cbp->GetHeight() + 1;

		sldr = (Slider*)dialogWindow->AddChild(
			new Slider("Red", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_red, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)dialogWindow->AddChild(
			new Slider("Green", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_green, 275));
		y += sldr->GetHeight() + 1;

		sldr = (Slider*)dialogWindow->AddChild(
			new Slider("Blue", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_blue, 275));
		y += sldr->GetHeight() + 1;

		cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Override Gloss", 2, y));
		cbp->SetBool(&Gloss_override_set);
		y += cbp->GetHeight() + 1;

		sldr = (Slider*)dialogWindow->AddChild(
			new Slider("Gloss", 0.0f, 255.0f, 0, y + 2, labviewer_set_material_override_specular_gloss, 275));
		y += sldr->GetHeight() + 1;
	}

}