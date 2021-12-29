#include "lab/labv2_internal.h"
#include "lab/dialogs/render_options.h"
#include "lighting/lighting_profiles.h"

void set_model_rotation_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Flags.set(ManagerFlags::ModelRotationEnabled, !value);
}

void set_show_insignia_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowInsignia, !value);
}

void set_show_damage_lightning_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowDamageLightning, !value);
}

void set_rotate_subsys_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::RotateSubsystems, !value);
}

void set_post_proc_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::HidePostProcessing, !value);
}

void set_diffuse_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoDiffuseMap, !value);
}

void set_glow_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoGlowMap, !value);
}

void set_spec_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoSpecularMap, !value);
}

void set_reflect_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoReflectMap, !value);
}

void set_env_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoEnvMap, !value);
}

void set_norm_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoNormalMap, !value);
}

void set_height_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoHeightMap, !value);
}

void set_misc_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoMiscMap, !value);
}

void set_ao_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoAOMap, !value);
}

void set_glowpoint_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoGlowpoints, !value);
}

void set_wireframe_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowWireframe, !value);
}

void set_renderlight_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::NoLighting, !value);
}

void set_fulldetail_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowFullDetail, !value);
}

void set_thrusters_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowThrusters, !value);
}

void set_afterburner_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowAfterburners, !value);
}

void set_weapons_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->loadWeapons();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowWeapons, !value);
}

void set_emissive_flag(Checkbox* caller) {
	auto value = caller->GetChecked();

	getLabManager()->Renderer->setRenderFlag(LabRenderFlag::ShowEmissiveLighting, !value);
}

void set_ambient_factor(Slider* caller) {
	auto value = caller->GetSliderValue();

	getLabManager()->Renderer->setAmbientFactor(fl2i(value));
}

void set_static_light_factor(Slider* caller) {
	auto value = caller->GetSliderValue();

	getLabManager()->Renderer->setDirectionalFactor(value);
}

void set_bloom(Slider* caller) {
	auto value = caller->GetSliderValue();

	getLabManager()->Renderer->setBloomLevel(fl2i(value));
}

void set_exposure(Slider* caller) {
	auto value = caller->GetSliderValue();

	getLabManager()->Renderer->setExposureLevel(value);
}

void set_ppc_ts(Slider* caller) {
	auto value = caller->GetSliderValue();
	auto ppcv = lighting_profile::lab_get_ppc();
	ppcv.toe_strength = value;
	getLabManager()->Renderer->setPPCValues(ppcv);
}

void set_ppc_tl(Slider* caller) {
	auto value = caller->GetSliderValue();
	auto ppcv = lighting_profile::lab_get_ppc();
	ppcv.toe_length = value;
	getLabManager()->Renderer->setPPCValues(ppcv);
}

void set_ppc_sa(Slider* caller) {
	auto value = caller->GetSliderValue();
	auto ppcv = lighting_profile::lab_get_ppc();
	ppcv.shoulder_angle = value;
	getLabManager()->Renderer->setPPCValues(ppcv);
}

void set_ppc_sl(Slider* caller) {
	auto value = caller->GetSliderValue();
	auto ppcv = lighting_profile::lab_get_ppc();
	ppcv.shoulder_length = value;
	getLabManager()->Renderer->setPPCValues(ppcv);
}

void set_ppc_ss(Slider* caller) {
	auto value = caller->GetSliderValue();
	auto ppcv = lighting_profile::lab_get_ppc();
	ppcv.shoulder_strength = value;
	getLabManager()->Renderer->setPPCValues(ppcv);
}
void change_detail_texture(Tree* caller) {
	auto detail = (TextureQuality)caller->GetSelectedItem()->GetData();

	getLabManager()->Renderer->setTextureQuality(detail);
}

void change_aa_setting(Tree* caller) {
	auto setting = (AntiAliasMode)caller->GetSelectedItem()->GetData();

	LabRenderer::setAAMode(setting);
}

void change_tonemapper_setting(Tree* caller) {
	auto setting = (TonemapperAlgorithm)caller->GetSelectedItem()->GetData();

	LabRenderer::setTonemapper(setting);
}

void RenderOptions::open(Button* /*caller*/) {
	if (dialogWindow != nullptr)
		return;

	dialogWindow = (DialogWindow*)getLabManager()->Screen->Add(new DialogWindow("Render Options", gr_screen.center_offset_x + gr_screen.center_w - 300, gr_screen.center_offset_y + 200));
	Assert(Opener != nullptr);
	dialogWindow->SetOwner(Opener->getDialog());

	dialogWindow->DeleteChildren();

	auto y = 0;
	auto cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Enable Model Rotation", 2, y, set_model_rotation_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Show Insignia", 2, y, set_show_insignia_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Show Damage lightning", 2, y, set_show_damage_lightning_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Rotate Subsystems", 2, y, set_rotate_subsys_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Hide Post Processing", 2, y, set_post_proc_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Diffuse Map", 2, y, set_diffuse_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Glow Map", 2, y, set_glow_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Specular Map", 2, y, set_spec_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Reflection Map", 2, y, set_reflect_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Environment Map", 2, y, set_env_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Normal Map", 2, y, set_norm_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Height Map", 2, y, set_height_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Misc Map", 2, y, set_misc_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Ambient Occlusion Map", 2, y, set_ao_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("No Glowpoints", 2, y, set_glowpoint_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Render as wireframe", 2, y, set_wireframe_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Render without light", 2, y, set_renderlight_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Show full detail", 2, y, set_fulldetail_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Show Thrusters", 2, y, set_thrusters_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Show Afterburner", 2, y, set_afterburner_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Show Weapons", 2, y, set_weapons_flag));
	y += cbp->GetHeight() + 2;

	cbp = (Checkbox*)dialogWindow->AddChild(new Checkbox("Render with emissive lighting", 2, y, set_emissive_flag));
	y += cbp->GetHeight() + 2;

	auto ambient_sldr = new Slider("Ambient Factor", 0, 128, 0, y + 2, set_ambient_factor, dialogWindow->GetWidth());
	ambient_sldr->SetSliderValue((float)Cmdline_ambient_factor);
	dialogWindow->AddChild(ambient_sldr);
	y += ambient_sldr->GetHeight() + 2;

	auto direct_sldr = new Slider("Direct. Lights", 0.0f, 2.0f, 0, y + 2, set_static_light_factor, dialogWindow->GetWidth());
	direct_sldr->SetSliderValue(static_light_factor);
	dialogWindow->AddChild(direct_sldr);
	y += direct_sldr->GetHeight() + 2;

	auto bloom_sldr = new Slider("Bloom", 0, 200, 0, y + 2, set_bloom, dialogWindow->GetWidth());
	bloom_sldr->SetSliderValue((float)gr_bloom_intensity());
	dialogWindow->AddChild(bloom_sldr);
	y += bloom_sldr->GetHeight() + 2;

	auto exposure_sldr= new Slider("Exposure", 0, 8, 0, y + 2, set_exposure, dialogWindow->GetWidth());
	exposure_sldr->SetSliderValue(lighting_profile::current_exposure());
	dialogWindow->AddChild(exposure_sldr);
	y += exposure_sldr->GetHeight() + 2;
	auto ppcv = lighting_profile::lab_get_ppc();

	auto ppcts_sldr= new Slider("PPC: Toe Strength", 0, 1, 0, y + 2, set_ppc_ts, dialogWindow->GetWidth());
	ppcts_sldr->SetSliderValue(ppcv.toe_strength);
	dialogWindow->AddChild(ppcts_sldr);
	y += ppcts_sldr->GetHeight() + 2;

	auto ppctl_sldr= new Slider("PPC: Toe Length", 0, 1, 0, y + 2, set_ppc_tl, dialogWindow->GetWidth());
	ppcts_sldr->SetSliderValue(ppcv.toe_length);
	dialogWindow->AddChild(ppctl_sldr);
	y += ppctl_sldr->GetHeight() + 2;

	auto ppcsa_sldr= new Slider("PPC: Shoudler Angle", 0, 1, 0, y + 2, set_ppc_sa, dialogWindow->GetWidth());
	ppcsa_sldr->SetSliderValue(ppcv.shoulder_angle);
	dialogWindow->AddChild(ppcsa_sldr);
	y += ppcsa_sldr->GetHeight() + 2;

	auto ppcsl_sldr= new Slider("PPC: Shoulder Length", 0, 10, 0, y + 2, set_ppc_sl, dialogWindow->GetWidth());
	ppcsl_sldr->SetSliderValue(ppcv.shoulder_length);
	dialogWindow->AddChild(ppcsl_sldr);
	y += ppcsl_sldr->GetHeight() + 2;

	auto ppcss_sldr= new Slider("PPC: Shoulder Strength", 0, 1, 0, y + 2, set_ppc_ss, dialogWindow->GetWidth());
	ppcss_sldr->SetSliderValue(ppcv.shoulder_strength);
	dialogWindow->AddChild(ppcss_sldr);
	y += ppcss_sldr->GetHeight() + 2;
	// start tree
	auto cmp = (Tree*)dialogWindow->AddChild(new Tree("Detail Options Tree", 0, y + 2, nullptr, dialogWindow->GetWidth()));

	// 3d hardware texture slider options
	auto ctip = cmp->AddItem(nullptr, "3D Hardware Textures", 0, false);

	cmp->AddItem(ctip, "Minimum", (int)TextureQuality::Minimum, false, change_detail_texture);
	cmp->AddItem(ctip, "Low",	  (int)TextureQuality::Low,	    false, change_detail_texture);
	cmp->AddItem(ctip, "Medium",  (int)TextureQuality::Medium,  false, change_detail_texture);
	cmp->AddItem(ctip, "High",    (int)TextureQuality::High,	false, change_detail_texture);
	cmp->AddItem(ctip, "Maximum", (int)TextureQuality::Maximum, false, change_detail_texture);

	auto aa_header = cmp->AddItem(nullptr, "AA Setting", 0, false);

	cmp->AddItem(aa_header, "None",        (int)AntiAliasMode::None,        false, change_aa_setting);
	cmp->AddItem(aa_header, "FXAA Low",    (int)AntiAliasMode::FXAA_Low,    false, change_aa_setting);
	cmp->AddItem(aa_header, "FXAA Medium", (int)AntiAliasMode::FXAA_Medium, false, change_aa_setting);
	cmp->AddItem(aa_header, "FXAA High",   (int)AntiAliasMode::FXAA_High,   false, change_aa_setting);
	cmp->AddItem(aa_header, "SMAA Low",    (int)AntiAliasMode::SMAA_Low,    false, change_aa_setting);
	cmp->AddItem(aa_header, "SMAA Medium", (int)AntiAliasMode::SMAA_Medium, false, change_aa_setting);
	cmp->AddItem(aa_header, "SMAA High",   (int)AntiAliasMode::SMAA_High,   false, change_aa_setting);
	cmp->AddItem(aa_header, "SMAA Ultra",  (int)AntiAliasMode::SMAA_Ultra,  false, change_aa_setting);

	auto tonemapper = cmp->AddItem(nullptr, "Tonemapper", 0, false);
	cmp->AddItem(tonemapper, "Linear", (int)TonemapperAlgorithm::tnm_Linear, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "Uncharted", (int)TonemapperAlgorithm::tnm_Uncharted, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "ACES", (int)TonemapperAlgorithm::tnm_Aces, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "ACES Approximate", (int)TonemapperAlgorithm::tnm_Aces_Approx, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "Cineon", (int)TonemapperAlgorithm::tnm_Cineon, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "Piecewise Power Curve", (int)TonemapperAlgorithm::tnm_PPC, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "Piecewise Power Curve(RGB)", (int)TonemapperAlgorithm::tnm_PPC_RGB, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "Reinhard Extended", (int)TonemapperAlgorithm::tnm_Reinhard_Extended, false, change_tonemapper_setting );
	cmp->AddItem(tonemapper, "Reinhard Jodie", (int)TonemapperAlgorithm::tnm_Reinhard_Jodie, false, change_tonemapper_setting );

}