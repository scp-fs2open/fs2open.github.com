// captureState() and restoreState() for VolumetricNebulaDialogModel.
// Snapshots The_mission.volumetrics from the live mission globals so that
// undo/redo can restore it without reopening the dialog.
// No sexp fields — serialization is complete.

#include <mission/dialogs/VolumetricNebulaDialogModel.h>

#include <mission/missionparse.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray VolumetricNebulaDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	const bool hasVol = The_mission.volumetrics.has_value();
	ds << hasVol;
	if (!hasVol)
		return data;

	const volumetric_nebula& v = *The_mission.volumetrics;

	ds << QString::fromStdString(v.hullPof);
	ds << v.pos.xyz.x << v.pos.xyz.y << v.pos.xyz.z;

	const auto& [cr, cg, cb] = v.nebulaColor;
	ds << cr << cg << cb;

	ds << v.doEdgeSmoothing;
	ds << static_cast<qint32>(v.steps);
	ds << static_cast<qint32>(v.globalLightSteps);
	ds << static_cast<qint32>(v.resolution);
	ds << static_cast<qint32>(v.oversampling);
	ds << v.smoothing;
	ds << static_cast<qint32>(v.noiseResolution);

	ds << v.opacityDistance;
	ds << v.alphaLim;

	ds << v.emissiveSpread;
	ds << v.emissiveIntensity;
	ds << v.emissiveFalloff;

	ds << v.henyeyGreensteinCoeff;
	ds << v.globalLightDistanceFactor;

	ds << v.noiseActive;
	const auto& [ns1, ns2] = v.noiseScale;
	ds << ns1 << ns2;

	const bool hasFunc1 = v.noiseColorFunc1.has_value();
	ds << hasFunc1;
	if (hasFunc1) ds << QString::fromStdString(*v.noiseColorFunc1);

	const bool hasFunc2 = v.noiseColorFunc2.has_value();
	ds << hasFunc2;
	if (hasFunc2) ds << QString::fromStdString(*v.noiseColorFunc2);

	const auto& [nr, ng, nb] = v.noiseColor;
	ds << nr << ng << nb;
	ds << v.noiseColorIntensity;

	ds << v.enabled;

	return data;
}

void VolumetricNebulaDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	bool hasVol;
	ds >> hasVol;
	if (!hasVol) {
		The_mission.volumetrics.reset();
		return;
	}

	if (!The_mission.volumetrics)
		The_mission.volumetrics.emplace();

	volumetric_nebula& v = *The_mission.volumetrics;

	QString hullPof;
	ds >> hullPof;
	v.hullPof = hullPof.toStdString();

	ds >> v.pos.xyz.x >> v.pos.xyz.y >> v.pos.xyz.z;

	float cr, cg, cb;
	ds >> cr >> cg >> cb;
	v.nebulaColor = std::make_tuple(cr, cg, cb);

	ds >> v.doEdgeSmoothing;
	qint32 steps, glSteps, res, oversamp, noiseRes;
	ds >> steps >> glSteps >> res >> oversamp >> noiseRes;
	v.steps = static_cast<int>(steps);
	v.globalLightSteps = static_cast<int>(glSteps);
	v.resolution = static_cast<int>(res);
	v.oversampling = static_cast<int>(oversamp);
	ds >> v.smoothing;
	v.noiseResolution = static_cast<int>(noiseRes);

	ds >> v.opacityDistance;
	ds >> v.alphaLim;

	ds >> v.emissiveSpread;
	ds >> v.emissiveIntensity;
	ds >> v.emissiveFalloff;

	ds >> v.henyeyGreensteinCoeff;
	ds >> v.globalLightDistanceFactor;

	ds >> v.noiseActive;
	float ns1, ns2;
	ds >> ns1 >> ns2;
	v.noiseScale = std::make_tuple(ns1, ns2);

	bool hasFunc1;
	ds >> hasFunc1;
	if (hasFunc1) {
		QString f;
		ds >> f;
		v.noiseColorFunc1 = f.toStdString();
	} else {
		v.noiseColorFunc1 = std::nullopt;
	}

	bool hasFunc2;
	ds >> hasFunc2;
	if (hasFunc2) {
		QString f;
		ds >> f;
		v.noiseColorFunc2 = f.toStdString();
	} else {
		v.noiseColorFunc2 = std::nullopt;
	}

	float nr, ng, nb;
	ds >> nr >> ng >> nb;
	v.noiseColor = std::make_tuple(nr, ng, nb);
	ds >> v.noiseColorIntensity;

	ds >> v.enabled;
}

} // namespace fso::fred::dialogs
