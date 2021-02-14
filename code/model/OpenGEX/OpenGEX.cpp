//
// This file is part of the Terathon OpenGEX Import Template, by Eric Lengyel.
// Copyright 2013-2021, Terathon Software LLC
//
// This software is licensed under the GNU General Public License version 3.
// Separate proprietary licenses are available from Terathon Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
//

#include "freespace.h"
#include "OpenGEX.h"
#include "tracing/tracing.h"


using namespace OpenGEX;


OpenGexStructure::OpenGexStructure(StructureType type) : Structure(type)
{
}

OpenGexStructure::~OpenGexStructure()
{
}


MetricStructure::MetricStructure() : OpenGexStructure(kStructureMetric)
{
}

MetricStructure::~MetricStructure()
{
}

bool MetricStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "key")
	{
		*type = kDataString;
		*value = &metricKey;
		return (true);
	}

	return (false);
}

bool MetricStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kDataFloat) || (type == kDataString))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult MetricStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	if (metricKey == "distance")
	{
		if (structure->GetStructureType() != kDataFloat)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		if (dataStructure->GetDataElementCount() != 1)
		{
			return (kDataInvalidDataFormat);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetDistanceScale(dataStructure->GetDataElement(0));
	}
	else if (metricKey == "angle")
	{
		if (structure->GetStructureType() != kDataFloat)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		if (dataStructure->GetDataElementCount() != 1)
		{
			return (kDataInvalidDataFormat);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetAngleScale(dataStructure->GetDataElement(0));
	}
	else if (metricKey == "time")
	{
		if (structure->GetStructureType() != kDataFloat)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		if (dataStructure->GetDataElementCount() != 1)
		{
			return (kDataInvalidDataFormat);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetTimeScale(dataStructure->GetDataElement(0));
	}
	else if (metricKey == "up")
	{
		if (structure->GetStructureType() != kDataString)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<StringDataType> *dataStructure = static_cast<const DataStructure<StringDataType> *>(structure);
		if (dataStructure->GetDataElementCount() != 1)
		{
			return (kDataInvalidDataFormat);
		}

		const String<>& string = dataStructure->GetDataElement(0);
		if ((string != "z") && (string != "y"))
		{
			return (kDataOpenGexInvalidUpDirection);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetUpDirection(string);
	}
	else if (metricKey == "forward")
	{
		if (structure->GetStructureType() != kDataString)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<StringDataType> *dataStructure = static_cast<const DataStructure<StringDataType> *>(structure);
		if (dataStructure->GetDataElementCount() != 1)
		{
			return (kDataInvalidDataFormat);
		}

		const String<>& string = dataStructure->GetDataElement(0);
		if ((string != "x") && (string != "y") && (string != "z") && (string != "-x") && (string != "-y") && (string != "-z"))
		{
			return (kDataOpenGexInvalidForwardDirection);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetForwardDirection(string);
	}
	else if (metricKey == "red")
	{
		if (structure->GetStructureType() != kDataFloat)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		if ((dataStructure->GetArraySize() != 2) || (dataStructure->GetDataElementCount() != 2))
		{
			return (kDataInvalidDataFormat);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetRedChromaticity(Vector2D(dataStructure->GetDataElement(0), dataStructure->GetDataElement(1)));
	}
	else if (metricKey == "green")
	{
		if (structure->GetStructureType() != kDataFloat)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		if ((dataStructure->GetArraySize() != 2) || (dataStructure->GetDataElementCount() != 2))
		{
			return (kDataInvalidDataFormat);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetGreenChromaticity(Vector2D(dataStructure->GetDataElement(0), dataStructure->GetDataElement(1)));
	}
	else if (metricKey == "blue")
	{
		if (structure->GetStructureType() != kDataFloat)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		if ((dataStructure->GetArraySize() != 2) || (dataStructure->GetDataElementCount() != 2))
		{
			return (kDataInvalidDataFormat);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetBlueChromaticity(Vector2D(dataStructure->GetDataElement(0), dataStructure->GetDataElement(1)));
	}
	else if (metricKey == "white")
	{
		if (structure->GetStructureType() != kDataFloat)
		{
			return (kDataInvalidDataFormat);
		}

		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		if ((dataStructure->GetArraySize() != 2) || (dataStructure->GetDataElementCount() != 2))
		{
			return (kDataInvalidDataFormat);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->SetWhiteChromaticity(Vector2D(dataStructure->GetDataElement(0), dataStructure->GetDataElement(1)));
	}

	return (kDataOkay);
}


NameStructure::NameStructure() : OpenGexStructure(kStructureName)
{
}

NameStructure::~NameStructure()
{
}

bool NameStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataString)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult NameStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<StringDataType> *dataStructure = static_cast<const DataStructure<StringDataType> *>(structure);
	if (dataStructure->GetDataElementCount() != 1)
	{
		return (kDataInvalidDataFormat);
	}

	name = dataStructure->GetDataElement(0);
	return (kDataOkay);
}


ObjectRefStructure::ObjectRefStructure() : OpenGexStructure(kStructureObjectRef)
{
	targetStructure = nullptr;
}

ObjectRefStructure::~ObjectRefStructure()
{
}

bool ObjectRefStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataRef)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult ObjectRefStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<RefDataType> *dataStructure = static_cast<const DataStructure<RefDataType> *>(structure);
	if (dataStructure->GetDataElementCount() != 0)
	{
		Structure *objectStructure = dataDescription->FindStructure(dataStructure->GetDataElement(0));
		if (objectStructure)
		{
			targetStructure = objectStructure;
			return (kDataOkay);
		}
	}

	return (kDataBrokenRef);
}


MaterialRefStructure::MaterialRefStructure() : OpenGexStructure(kStructureMaterialRef)
{
	materialIndex = 0;
	targetStructure = nullptr;
}

MaterialRefStructure::~MaterialRefStructure()
{
}

bool MaterialRefStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "index")
	{
		*type = kDataUInt32;
		*value = &materialIndex;
		return (true);
	}

	return (false);
}

bool MaterialRefStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataRef)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult MaterialRefStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<RefDataType> *dataStructure = static_cast<const DataStructure<RefDataType> *>(structure);
	if (dataStructure->GetDataElementCount() != 0)
	{
		const Structure *materialStructure = dataDescription->FindStructure(dataStructure->GetDataElement(0));
		if (materialStructure)
		{
			if (materialStructure->GetStructureType() != kStructureMaterial)
			{
				return (kDataOpenGexInvalidMaterialRef);
			}

			targetStructure = static_cast<const MaterialStructure *>(materialStructure);
			return (kDataOkay);
		}
	}

	return (kDataBrokenRef);
}


AnimatableStructure::AnimatableStructure(StructureType type) : OpenGexStructure(type)
{
}

AnimatableStructure::~AnimatableStructure()
{
}


MatrixStructure::MatrixStructure(StructureType type) : AnimatableStructure(type)
{
	SetBaseStructureType(kStructureMatrix);

	objectFlag = false;
	matrixValue.SetIdentity();
}

MatrixStructure::~MatrixStructure()
{
}

bool MatrixStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "object")
	{
		*type = kDataBool;
		*value = &objectFlag;
		return (true);
	}

	return (false);
}


TransformStructure::TransformStructure() : MatrixStructure(kStructureTransform)
{
}

TransformStructure::~TransformStructure()
{
}

bool TransformStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);
		uint32 arraySize = primitiveStructure->GetArraySize();
		return ((arraySize == 16) || (arraySize == 12) || (arraySize == 9) || (arraySize == 6) || (arraySize == 4));
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult TransformStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	const float *data = &dataStructure->GetDataElement(0);
	int32 arraySize = dataStructure->GetArraySize();

	int32 transformCount = dataStructure->GetDataElementCount() / arraySize;
	if (transformCount == 0)
	{
		return (kDataInvalidDataFormat);
	}

	if (arraySize == 16)
	{
		const Transform4D *transform = reinterpret_cast<const Transform4D *>(data);
		for (machine a = 0; a < transformCount; a++)
		{
			transformArray.AppendArrayElement(transform[a]);
		}
	}
	else if (arraySize == 12)
	{
		for (machine a = 0; a < transformCount; a++)
		{
			transformArray.AppendArrayElement(Transform4D(data[0], data[3], data[6], data[9], data[1], data[4], data[7], data[10], data[2], data[5], data[8], data[11]));
			data += 12;
		}
	}
	else if (arraySize == 9)
	{
		for (machine a = 0; a < transformCount; a++)
		{
			transformArray.AppendArrayElement(Transform4D(data[0], data[3], data[6], 0.0F, data[1], data[4], data[7], 0.0F, data[2], data[5], data[8], 0.0F));
			data += 9;
		}
	}
	else if (arraySize == 6)
	{
		for (machine a = 0; a < transformCount; a++)
		{
			transformArray.AppendArrayElement(Transform4D(data[0], data[2], 0.0F, data[4], data[1], data[3], 0.0F, data[5], 0.0F, 0.0F, 1.0F, 0.0F));
			data += 6;
		}
	}
	else
	{
		for (machine a = 0; a < transformCount; a++)
		{
			transformArray.AppendArrayElement(Transform4D(data[0], data[2], 0.0F, 0.0F, data[1], data[3], 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F));
			data += 4;
		}
	}

	matrixValue = transformArray[0];
	return (kDataOkay);
}

void TransformStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, const float *data)
{
	matrixValue = *reinterpret_cast<const Transform4D *>(data);
}


TranslationStructure::TranslationStructure() :
		MatrixStructure(kStructureTranslation),
		translationKind("xyz")
{
}

TranslationStructure::~TranslationStructure()
{
}

bool TranslationStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "kind")
	{
		*type = kDataString;
		*value = &translationKind;
		return (true);
	}

	return (false);
}

bool TranslationStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);
		uint32 arraySize = primitiveStructure->GetArraySize();
		return ((arraySize == 0) || (arraySize == 3));
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult TranslationStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	uint32 arraySize = dataStructure->GetArraySize();

	if ((translationKind == "x") || (translationKind == "y") || (translationKind == "z"))
	{
		if ((arraySize != 0) || (dataStructure->GetDataElementCount() != 1))
		{
			return (kDataInvalidDataFormat);
		}
	}
	else if (translationKind == "xyz")
	{
		if ((arraySize != 3) || (dataStructure->GetDataElementCount() != 3))
		{
			return (kDataInvalidDataFormat);
		}
	}
	else
	{
		return (kDataOpenGexInvalidTranslationKind);
	}

	TranslationStructure::UpdateAnimation(static_cast<const OpenGexDataDescription *>(dataDescription), &dataStructure->GetDataElement(0));
	return (kDataOkay);
}

void TranslationStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, const float *data)
{
	if (translationKind == "x")
	{
		matrixValue.SetTranslation(data[0], 0.0F, 0.0F);
	}
	else if (translationKind == "y")
	{
		matrixValue.SetTranslation(0.0F, data[0], 0.0F);
	}
	else if (translationKind == "z")
	{
		matrixValue.SetTranslation(0.0F, 0.0F, data[0]);
	}
	else
	{
		matrixValue.SetTranslation(data[0], data[1], data[2]);
	}
}


RotationStructure::RotationStructure() :
		MatrixStructure(kStructureRotation),
		rotationKind("axis")
{
}

RotationStructure::~RotationStructure()
{
}

bool RotationStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "kind")
	{
		*type = kDataString;
		*value = &rotationKind;
		return (true);
	}

	return (false);
}

bool RotationStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);
		uint32 arraySize = primitiveStructure->GetArraySize();
		return ((arraySize == 0) || (arraySize == 4));
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult RotationStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	uint32 arraySize = dataStructure->GetArraySize();

	if ((rotationKind == "x") || (rotationKind == "y") || (rotationKind == "z"))
	{
		if ((arraySize != 0) || (dataStructure->GetDataElementCount() != 1))
		{
			return (kDataInvalidDataFormat);
		}
	}
	else if ((rotationKind == "axis") || (rotationKind == "quaternion"))
	{
		if ((arraySize != 4) || (dataStructure->GetDataElementCount() != 4))
		{
			return (kDataInvalidDataFormat);
		}
	}
	else
	{
		return (kDataOpenGexInvalidRotationKind);
	}

	RotationStructure::UpdateAnimation(static_cast<const OpenGexDataDescription *>(dataDescription), &dataStructure->GetDataElement(0));
	return (kDataOkay);
}

void RotationStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, const float *data)
{
	float scale = dataDescription->GetAngleScale();

	if (rotationKind == "x")
	{
		matrixValue = Transform4D::MakeRotationX(data[0] * scale);
	}
	else if (rotationKind == "y")
	{
		matrixValue = Transform4D::MakeRotationY(data[0] * scale);
	}
	else if (rotationKind == "z")
	{
		matrixValue = Transform4D::MakeRotationZ(data[0] * scale);
	}
	else if (rotationKind == "axis")
	{
		matrixValue = Transform4D::MakeRotation(data[0] * scale, Bivector3D(data[1], data[2], data[3]).Normalize());
	}
	else
	{
		matrixValue.matrix3D = Quaternion(data[0], data[1], data[2], data[3]).Normalize().GetRotationMatrix();
		matrixValue[3].Set(0.0F, 0.0F, 0.0F);
	}
}


ScaleStructure::ScaleStructure() :
		MatrixStructure(kStructureScale),
		scaleKind("xyz")
{
}

ScaleStructure::~ScaleStructure()
{
}

bool ScaleStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "kind")
	{
		*type = kDataString;
		*value = &scaleKind;
		return (true);
	}

	return (false);
}

bool ScaleStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);
		uint32 arraySize = primitiveStructure->GetArraySize();
		return ((arraySize == 0) || (arraySize == 3));
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult ScaleStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	uint32 arraySize = dataStructure->GetArraySize();

	if ((scaleKind == "x") || (scaleKind == "y") || (scaleKind == "z"))
	{
		if ((arraySize != 0) || (dataStructure->GetDataElementCount() != 1))
		{
			return (kDataInvalidDataFormat);
		}
	}
	else if (scaleKind == "xyz")
	{
		if ((arraySize != 3) || (dataStructure->GetDataElementCount() != 3))
		{
			return (kDataInvalidDataFormat);
		}
	}
	else
	{
		return (kDataOpenGexInvalidScaleKind);
	}

	ScaleStructure::UpdateAnimation(static_cast<const OpenGexDataDescription *>(dataDescription), &dataStructure->GetDataElement(0));
	return (kDataOkay);
}

void ScaleStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, const float *data)
{
	if (scaleKind == "x")
	{
		matrixValue = Transform4D::MakeScaleX(data[0]);
	}
	else if (scaleKind == "y")
	{
		matrixValue = Transform4D::MakeScaleY(data[0]);
	}
	else if (scaleKind == "z")
	{
		matrixValue = Transform4D::MakeScaleZ(data[0]);
	}
	else if (scaleKind == "xyz")
	{
		matrixValue = Transform4D::MakeScale(data[0], data[1], data[2]);
	}
}


MorphWeightStructure::MorphWeightStructure() : AnimatableStructure(kStructureMorphWeight)
{
	morphIndex = 0;
	morphWeight = 0.0F;
}

MorphWeightStructure::~MorphWeightStructure()
{
}

bool MorphWeightStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "index")
	{
		*type = kDataUInt32;
		*value = &morphIndex;
		return (true);
	}

	return (false);
}

bool MorphWeightStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		uint32 arraySize = dataStructure->GetArraySize();
		return (arraySize == 0);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult MorphWeightStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	if (dataStructure->GetDataElementCount() == 1)
	{
		morphWeight = dataStructure->GetDataElement(0);
	}
	else
	{
		return (kDataInvalidDataFormat);
	}

	// Do application-specific morph weight processing here.

	return (kDataOkay);
}

void MorphWeightStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, const float *data)
{
	morphWeight = data[0];
}


NodeStructure::NodeStructure() : OpenGexStructure(kStructureNode)
{
	SetBaseStructureType(kStructureNode);
}

NodeStructure::NodeStructure(StructureType type) : OpenGexStructure(type)
{
	SetBaseStructureType(kStructureNode);
}

NodeStructure::~NodeStructure()
{
}

bool NodeStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetBaseStructureType();
	if ((type == kStructureNode) || (type == kStructureMatrix))
	{
		return (true);
	}

	type = structure->GetStructureType();
	if ((type == kStructureName) || (type == kStructureAnimation))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult NodeStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	const Structure *structure = GetFirstSubstructure(kStructureName);
	if (structure)
	{
		if (GetLastSubstructure(kStructureName) != structure)
		{
			return (kDataExtraneousSubstructure);
		}

		nodeName = static_cast<const NameStructure *>(structure)->GetName();
	}
	else
	{
		nodeName = nullptr;
	}

	// Do application-specific node processing here.

	return (kDataOkay);
}

const ObjectStructure *NodeStructure::GetObjectStructure(void) const
{
	return (nullptr);
}

void NodeStructure::CalculateNodeTransforms(const OpenGexDataDescription *dataDescription)
{
	nodeTransform.SetIdentity();
	objectTransform.SetIdentity();

	const ObjectStructure *objectStructure = GetObjectStructure();

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetBaseStructureType() == kStructureMatrix)
		{
			const MatrixStructure *matrixStructure = static_cast<const MatrixStructure *>(structure);
			if (!matrixStructure->GetObjectFlag())
			{
				nodeTransform = nodeTransform * matrixStructure->GetMatrix();
			}
			else if (objectStructure)
			{
				objectTransform = objectTransform * matrixStructure->GetMatrix();
			}
		}

		structure = structure->GetNextSubnode();
	}

	dataDescription->AdjustTransform(nodeTransform);
	dataDescription->AdjustTransform(objectTransform);

	inverseObjectTransform = Inverse(objectTransform);
}

void NodeStructure::UpdateNodeTransforms(const OpenGexDataDescription *dataDescription)
{
	CalculateNodeTransforms(dataDescription);

	Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetBaseStructureType() == kStructureNode)
		{
			static_cast<NodeStructure *>(structure)->UpdateNodeTransforms(dataDescription);
		}

		structure = structure->GetNextSubnode();
	}
}


BoneNodeStructure::BoneNodeStructure() : NodeStructure(kStructureBoneNode)
{
}

BoneNodeStructure::~BoneNodeStructure()
{
}


GeometryNodeStructure::GeometryNodeStructure() : NodeStructure(kStructureGeometryNode)
{
	// The first entry in each of the following arrays indicates whether the flag
	// is specified by a property. If true, then the second entry in the array
	// indicates the actual value that the property specified.

	visibleFlag[0] = false;
	shadowFlag[0] = false;
	motionBlurFlag[0] = false;
}

GeometryNodeStructure::~GeometryNodeStructure()
{
}

bool GeometryNodeStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "visible")
	{
		*type = kDataBool;
		*value = &visibleFlag[1];
		visibleFlag[0] = true;
		return (true);
	}

	if (identifier == "shadow")
	{
		*type = kDataBool;
		*value = &shadowFlag[1];
		shadowFlag[0] = true;
		return (true);
	}

	if (identifier == "motion_blur")
	{
		*type = kDataBool;
		*value = &motionBlurFlag[1];
		motionBlurFlag[0] = true;
		return (true);
	}

	return (false);
}

bool GeometryNodeStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kStructureObjectRef) || (type == kStructureMaterialRef) || (type == kStructureMorphWeight))
	{
		return (true);
	}

	return (NodeStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult GeometryNodeStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = NodeStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	bool objectFlag = false;
	bool materialFlag[256] = {false};
	int32 maxMaterialIndex = -1;

	Structure *structure = GetFirstSubnode();
	while (structure)
	{
		StructureType type = structure->GetStructureType();
		if (type == kStructureObjectRef)
		{
			if (objectFlag)
			{
				return (kDataExtraneousSubstructure);
			}

			objectFlag = true;

			Structure *objectStructure = static_cast<ObjectRefStructure *>(structure)->GetTargetStructure();
			if (objectStructure->GetStructureType() != kStructureGeometryObject)
			{
				return (kDataOpenGexInvalidObjectRef);
			}

			geometryObjectStructure = static_cast<GeometryObjectStructure *>(objectStructure);
		}
		else if (type == kStructureMaterialRef)
		{
			const MaterialRefStructure *materialRefStructure = static_cast<MaterialRefStructure *>(structure);

			uint32 index = materialRefStructure->GetMaterialIndex();
			if (index > 255)
			{
				// We only support up to 256 materials.
				return (kDataOpenGexMaterialIndexUnsupported);
			}

			if (materialFlag[index])
			{
				return (kDataOpenGexDuplicateMaterialRef);
			}

			materialFlag[index] = true;
			maxMaterialIndex = Max(maxMaterialIndex, int32(index));
		}
		else if (type == kStructureMorphWeight)
		{
			morphWeightList.AppendListElement(static_cast<MorphWeightStructure *>(structure));
		}

		structure = structure->GetNextSubnode();
	}

	if (!objectFlag)
	{
		return (kDataMissingSubstructure);
	}

	if (maxMaterialIndex >= 0)
	{
		for (machine a = 0; a <= maxMaterialIndex; a++)
		{
			if (!materialFlag[a])
			{
				return (kDataOpenGexMissingMaterialRef);
			}
		}

		materialStructureArray.SetArrayElementCount(maxMaterialIndex + 1);

		structure = GetFirstSubnode();
		while (structure)
		{
			if (structure->GetStructureType() == kStructureMaterialRef)
			{
				const MaterialRefStructure *materialRefStructure = static_cast<const MaterialRefStructure *>(structure);
				materialStructureArray[materialRefStructure->GetMaterialIndex()] = materialRefStructure->GetTargetStructure();
			}

			structure = structure->GetNextSubnode();
		}
	}

	// Do application-specific node processing here.

	return (kDataOkay);
}

const ObjectStructure *GeometryNodeStructure::GetObjectStructure(void) const
{
	return (geometryObjectStructure);
}

const MorphWeightStructure *GeometryNodeStructure::FindMorphWeightStructure(uint32 index) const
{
	const MorphWeightStructure *morphWeightStructure = morphWeightList.GetFirstListElement();
	while (morphWeightStructure)
	{
		if (morphWeightStructure->GetMorphIndex() == index)
		{
			return (morphWeightStructure);
		}

		morphWeightStructure = morphWeightStructure->GetNextListElement();
	}

	return (nullptr);
}


LightNodeStructure::LightNodeStructure() : NodeStructure(kStructureLightNode)
{
	// The first entry in the following array indicates whether the flag is
	// specified by a property. If true, then the second entry in the array
	// indicates the actual value that the property specified.

	shadowFlag[0] = false;
}

LightNodeStructure::~LightNodeStructure()
{
}

bool LightNodeStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "shadow")
	{
		*type = kDataBool;
		*value = &shadowFlag[1];
		shadowFlag[0] = true;
		return (true);
	}

	return (false);
}

bool LightNodeStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kStructureObjectRef)
	{
		return (true);
	}

	return (NodeStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult LightNodeStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = NodeStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	bool objectFlag = false;

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == kStructureObjectRef)
		{
			if (objectFlag)
			{
				return (kDataExtraneousSubstructure);
			}

			objectFlag = true;

			const Structure *objectStructure = static_cast<const ObjectRefStructure *>(structure)->GetTargetStructure();
			if (objectStructure->GetStructureType() != kStructureLightObject)
			{
				return (kDataOpenGexInvalidObjectRef);
			}

			lightObjectStructure = static_cast<const LightObjectStructure *>(objectStructure);
		}

		structure = structure->GetNextSubnode();
	}

	if (!objectFlag)
	{
		return (kDataMissingSubstructure);
	}

	// Do application-specific node processing here.

	return (kDataOkay);
}

const ObjectStructure *LightNodeStructure::GetObjectStructure(void) const
{
	return (lightObjectStructure);
}


CameraNodeStructure::CameraNodeStructure() : NodeStructure(kStructureCameraNode)
{
}

CameraNodeStructure::~CameraNodeStructure()
{
}

bool CameraNodeStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kStructureObjectRef)
	{
		return (true);
	}

	return (NodeStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult CameraNodeStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = NodeStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	bool objectFlag = false;

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == kStructureObjectRef)
		{
			if (objectFlag)
			{
				return (kDataExtraneousSubstructure);
			}

			objectFlag = true;

			const Structure *objectStructure = static_cast<const ObjectRefStructure *>(structure)->GetTargetStructure();
			if (objectStructure->GetStructureType() != kStructureCameraObject)
			{
				return (kDataOpenGexInvalidObjectRef);
			}

			cameraObjectStructure = static_cast<const CameraObjectStructure *>(objectStructure);
		}

		structure = structure->GetNextSubnode();
	}

	if (!objectFlag)
	{
		return (kDataMissingSubstructure);
	}

	// Do application-specific node processing here.

	return (kDataOkay);
}

const ObjectStructure *CameraNodeStructure::GetObjectStructure(void) const
{
	return (cameraObjectStructure);
}


VertexArrayStructure::VertexArrayStructure() : OpenGexStructure(kStructureVertexArray)
{
	attribIndex = 0;
	morphIndex = 0;

	arrayStorage = nullptr;
	floatStorage = nullptr;
}

VertexArrayStructure::~VertexArrayStructure()
{
	delete[] arrayStorage;
	delete[] floatStorage;
}

bool VertexArrayStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "attrib")
	{
		*type = kDataString;
		*value = &attribString;
		return (true);
	}

	if (identifier == "index")
	{
		*type = kDataUInt32;
		*value = &attribIndex;
		return (true);
	}

	if (identifier == "morph")
	{
		*type = kDataUInt32;
		*value = &morphIndex;
		return (true);
	}

	return (false);
}

bool VertexArrayStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kDataHalf) || (type == kDataFloat) || (type == kDataDouble))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult VertexArrayStructure::ProcessData(DataDescription *dataDescription)
{
	int32			elementCount;
	const float		*data;

	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);

	StructureType type = primitiveStructure->GetStructureType();
	if (type == kDataFloat)
	{
		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		elementCount = dataStructure->GetDataElementCount();
		data = &dataStructure->GetDataElement(0);
	}
	else if (type == kDataDouble)
	{
		const DataStructure<DoubleDataType> *dataStructure = static_cast<const DataStructure<DoubleDataType> *>(structure);
		elementCount = dataStructure->GetDataElementCount();

		float *floatElement = new float[elementCount];
		floatStorage = floatElement;
		data = floatElement;

		const double *doubleElement = &dataStructure->GetDataElement(0);
		for (machine a = 0; a < elementCount; a++)
		{
			floatElement[a] = float(doubleElement[a]);
		}
	}
	else // must be kDataHalf
	{
		const DataStructure<HalfDataType> *dataStructure = static_cast<const DataStructure<HalfDataType> *>(structure);
		elementCount = dataStructure->GetDataElementCount();

		float *floatElement = new float[elementCount];
		floatStorage = floatElement;
		data = floatElement;

		const Half *halfElement = &dataStructure->GetDataElement(0);
		for (machine a = 0; a < elementCount; a++)
		{
			floatElement[a] = halfElement[a];
		}
	}

	int32 arraySize = primitiveStructure->GetArraySize();
	vertexCount = elementCount / arraySize;
	componentCount = arraySize;
	vertexArrayData = data;

	if (attribString == "position")
	{
		if (arraySize == 3)
		{
			const OpenGexDataDescription *openGexDataDescription = static_cast<OpenGexDataDescription *>(dataDescription);
			float scale = openGexDataDescription->GetDistanceScale();
			char up = openGexDataDescription->GetUpDirection()[0];

			if ((scale != 1.0F) || (up != 'z'))
			{
				Transform4D		transform;

				if (up == 'z')
				{
					transform.Set(scale,  0.0F,  0.0F, 0.0F,
									0.0F, scale,  0.0F, 0.0F,
									0.0F,  0.0F, scale, 0.0F);
				}
				else
				{
					transform.Set(scale,  0.0F,   0.0F, 0.0F,
									0.0F,  0.0F, -scale, 0.0F,
									0.0F, scale,   0.0F, 0.0F);
				}

				arrayStorage = new char[vertexCount * sizeof(Point3D)];
				vertexArrayData = arrayStorage;

				const Point3D *inputPosition = reinterpret_cast<const Point3D *>(data);
				Point3D *outputPosition = reinterpret_cast<Point3D *>(arrayStorage);

				for (machine a = 0; a < vertexCount; a++)
				{
					outputPosition[a] = transform * inputPosition[a];
				}
			}
		}
	}
	else if ((attribString == "normal") || (attribString == "tangent") || (attribString == "bitangent"))
	{
		if (arraySize == 3)
		{
			const OpenGexDataDescription *openGexDataDescription = static_cast<OpenGexDataDescription *>(dataDescription);
			if (openGexDataDescription->GetUpDirection()[0] != 'z')
			{
				arrayStorage = new char[vertexCount * sizeof(Vector3D)];
				vertexArrayData = arrayStorage;

				const Vector3D *inputVector = reinterpret_cast<const Vector3D *>(data);
				Vector3D *outputVector = reinterpret_cast<Vector3D *>(arrayStorage);

				for (machine a = 0; a < vertexCount; a++)
				{
					const Vector3D& v = inputVector[a];
					outputVector[a].Set(v.x, -v.z, v.y);
				}
			}
		}
	}
	else if (attribString == "color")
	{
		OpenGexDataDescription *openGexDataDescription = static_cast<OpenGexDataDescription *>(dataDescription);

		if (arraySize == 3)
		{
			arrayStorage = new char[vertexCount * sizeof(ColorRGB)];
			vertexArrayData = arrayStorage;

			const ColorRGB *inputColor = reinterpret_cast<const ColorRGB *>(data);
			ColorRGB *outputColor = reinterpret_cast<ColorRGB *>(arrayStorage);

			for (machine a = 0; a < vertexCount; a++)
			{
				outputColor[a] = inputColor[a];
				openGexDataDescription->ConvertColor(outputColor[a]);
			}
		}
		else if (arraySize == 4)
		{
			arrayStorage = new char[vertexCount * sizeof(ColorRGBA)];
			vertexArrayData = arrayStorage;

			const ColorRGBA *inputColor = reinterpret_cast<const ColorRGBA *>(data);
			ColorRGBA *outputColor = reinterpret_cast<ColorRGBA *>(arrayStorage);

			for (machine a = 0; a < vertexCount; a++)
			{
				outputColor[a] = inputColor[a];
				openGexDataDescription->ConvertColor(outputColor[a].GetColorRGB());
			}
		}
	}

	return (kDataOkay);
}


IndexArrayStructure::IndexArrayStructure() : OpenGexStructure(kStructureIndexArray)
{
	materialIndex = 0;
	restartIndex = 0;
	frontFace = "ccw";
}

IndexArrayStructure::~IndexArrayStructure()
{
}

bool IndexArrayStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "material")
	{
		*type = kDataUInt32;
		*value = &materialIndex;
		return (true);
	}

	if (identifier == "restart")
	{
		*type = kDataUInt64;
		*value = &restartIndex;
		return (true);
	}

	if (identifier == "front")
	{
		*type = kDataString;
		*value = &frontFace;
		return (true);
	}

	return (false);
}

bool IndexArrayStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kDataUInt8) || (type == kDataUInt16) || (type == kDataUInt32) || (type == kDataUInt64))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult IndexArrayStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);
	if (primitiveStructure->GetArraySize() != 3)
	{
		return (kDataInvalidDataFormat);
	}

	// Do something with the index array here.

	return (kDataOkay);
}


BoneRefArrayStructure::BoneRefArrayStructure() : OpenGexStructure(kStructureBoneRefArray)
{
	boneNodeArray = nullptr;
}

BoneRefArrayStructure::~BoneRefArrayStructure()
{
	delete[] boneNodeArray;
}

bool BoneRefArrayStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataRef)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult BoneRefArrayStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<RefDataType> *dataStructure = static_cast<const DataStructure<RefDataType> *>(structure);
	boneCount = dataStructure->GetDataElementCount();

	if (boneCount != 0)
	{
		boneNodeArray = new const BoneNodeStructure *[boneCount];

		for (machine a = 0; a < boneCount; a++)
		{
			const StructureRef& reference = dataStructure->GetDataElement(a);
			const Structure *boneStructure = dataDescription->FindStructure(reference);
			if (!boneStructure)
			{
				return (kDataBrokenRef);
			}

			if (boneStructure->GetStructureType() != kStructureBoneNode)
			{
				return (kDataOpenGexInvalidBoneRef);
			}

			boneNodeArray[a] = static_cast<const BoneNodeStructure *>(boneStructure);
		}
	}

	return (kDataOkay);
}


BoneCountArrayStructure::BoneCountArrayStructure() : OpenGexStructure(kStructureBoneCountArray)
{
	arrayStorage = nullptr;
}

BoneCountArrayStructure::~BoneCountArrayStructure()
{
	delete[] arrayStorage;
}

bool BoneCountArrayStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kDataUInt8) || (type == kDataUInt16) || (type == kDataUInt32) || (type == kDataUInt64))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult BoneCountArrayStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);
	if (primitiveStructure->GetArraySize() != 0)
	{
		return (kDataInvalidDataFormat);
	}

	StructureType type = primitiveStructure->GetStructureType();
	if (type == kDataUInt16)
	{
		const DataStructure<UInt16DataType> *dataStructure = static_cast<const DataStructure<UInt16DataType> *>(primitiveStructure);
		vertexCount = dataStructure->GetDataElementCount();
		boneCountArray = &dataStructure->GetDataElement(0);
	}
	else if (type == kDataUInt8)
	{
		const DataStructure<UInt8DataType> *dataStructure = static_cast<const DataStructure<UInt8DataType> *>(primitiveStructure);
		vertexCount = dataStructure->GetDataElementCount();

		const uint8 *data = &dataStructure->GetDataElement(0);
		arrayStorage = new uint16[vertexCount];
		boneCountArray = arrayStorage;

		for (machine a = 0; a < vertexCount; a++)
		{
			arrayStorage[a] = data[a];
		}
	}
	else if (type == kDataUInt32)
	{
		const DataStructure<UInt32DataType> *dataStructure = static_cast<const DataStructure<UInt32DataType> *>(primitiveStructure);
		vertexCount = dataStructure->GetDataElementCount();

		const uint32 *data = &dataStructure->GetDataElement(0);
		arrayStorage = new uint16[vertexCount];
		boneCountArray = arrayStorage;

		for (machine a = 0; a < vertexCount; a++)
		{
			uint32 index = data[a];
			if (index > 65535)
			{
				// We only support 16-bit counts or smaller.
				return (kDataOpenGexIndexValueUnsupported);
			}

			arrayStorage[a] = uint16(index);
		}
	}
	else // must be 64-bit
	{
		const DataStructure<UInt64DataType> *dataStructure = static_cast<const DataStructure<UInt64DataType> *>(primitiveStructure);
		vertexCount = dataStructure->GetDataElementCount();

		const uint64 *data = &dataStructure->GetDataElement(0);
		arrayStorage = new uint16[vertexCount];
		boneCountArray = arrayStorage;

		for (machine a = 0; a < vertexCount; a++)
		{
			uint64 index = data[a];
			if (index > 65535)
			{
				// We only support 16-bit counts or smaller.
				return (kDataOpenGexIndexValueUnsupported);
			}

			arrayStorage[a] = uint16(index);
		}
	}

	return (kDataOkay);
}


BoneIndexArrayStructure::BoneIndexArrayStructure() : OpenGexStructure(kStructureBoneIndexArray)
{
	arrayStorage = nullptr;
}

BoneIndexArrayStructure::~BoneIndexArrayStructure()
{
	delete[] arrayStorage;
}

bool BoneIndexArrayStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kDataUInt8) || (type == kDataUInt16) || (type == kDataUInt32) || (type == kDataUInt64))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult BoneIndexArrayStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const PrimitiveStructure *primitiveStructure = static_cast<const PrimitiveStructure *>(structure);
	if (primitiveStructure->GetArraySize() != 0)
	{
		return (kDataInvalidDataFormat);
	}

	StructureType type = primitiveStructure->GetStructureType();
	if (type == kDataUInt16)
	{
		const DataStructure<UInt16DataType> *dataStructure = static_cast<const DataStructure<UInt16DataType> *>(primitiveStructure);
		boneIndexCount = dataStructure->GetDataElementCount();
		boneIndexArray = &dataStructure->GetDataElement(0);
	}
	else if (type == kDataUInt8)
	{
		const DataStructure<UInt8DataType> *dataStructure = static_cast<const DataStructure<UInt8DataType> *>(primitiveStructure);
		boneIndexCount = dataStructure->GetDataElementCount();

		const uint8 *data = &dataStructure->GetDataElement(0);
		arrayStorage = new uint16[boneIndexCount];
		boneIndexArray = arrayStorage;

		for (machine a = 0; a < boneIndexCount; a++)
		{
			arrayStorage[a] = data[a];
		}
	}
	else if (type == kDataUInt32)
	{
		const DataStructure<UInt32DataType> *dataStructure = static_cast<const DataStructure<UInt32DataType> *>(primitiveStructure);
		boneIndexCount = dataStructure->GetDataElementCount();

		const uint32 *data = &dataStructure->GetDataElement(0);
		arrayStorage = new uint16[boneIndexCount];
		boneIndexArray = arrayStorage;

		for (machine a = 0; a < boneIndexCount; a++)
		{
			uint32 index = data[a];
			if (index > 65535)
			{
				// We only support 16-bit indexes or smaller.
				return (kDataOpenGexIndexValueUnsupported);
			}

			arrayStorage[a] = uint16(index);
		}
	}
	else // must be 64-bit
	{
		const DataStructure<UInt64DataType> *dataStructure = static_cast<const DataStructure<UInt64DataType> *>(primitiveStructure);
		boneIndexCount = dataStructure->GetDataElementCount();

		const uint64 *data = &dataStructure->GetDataElement(0);
		arrayStorage = new uint16[boneIndexCount];
		boneIndexArray = arrayStorage;

		for (machine a = 0; a < boneIndexCount; a++)
		{
			uint64 index = data[a];
			if (index > 65535)
			{
				// We only support 16-bit indexes or smaller.
				return (kDataOpenGexIndexValueUnsupported);
			}

			arrayStorage[a] = uint16(index);
		}
	}

	return (kDataOkay);
}


BoneWeightArrayStructure::BoneWeightArrayStructure() : OpenGexStructure(kStructureBoneWeightArray)
{
}

BoneWeightArrayStructure::~BoneWeightArrayStructure()
{
}

bool BoneWeightArrayStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult BoneWeightArrayStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	if (dataStructure->GetArraySize() != 0)
	{
		return (kDataInvalidDataFormat);
	}

	boneWeightCount = dataStructure->GetDataElementCount();
	boneWeightArray = &dataStructure->GetDataElement(0);
	return (kDataOkay);
}


SkeletonStructure::SkeletonStructure() : OpenGexStructure(kStructureSkeleton)
{
}

SkeletonStructure::~SkeletonStructure()
{
}

bool SkeletonStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kStructureBoneRefArray) || (type == kStructureTransform))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult SkeletonStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	const Structure *structure = GetFirstSubstructure(kStructureBoneRefArray);
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubstructure(kStructureBoneRefArray) != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	boneRefArrayStructure = static_cast<const BoneRefArrayStructure *>(structure);

	structure = GetFirstSubstructure(kStructureTransform);
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubstructure(kStructureTransform) != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	transformStructure = static_cast<const TransformStructure *>(structure);

	if (boneRefArrayStructure->GetBoneCount() != transformStructure->GetTransformCount())
	{
		return (kDataOpenGexBoneCountMismatch);
	}

	return (kDataOkay);
}


SkinStructure::SkinStructure() : OpenGexStructure(kStructureSkin)
{
}

SkinStructure::~SkinStructure()
{
}

bool SkinStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kStructureTransform) || (type == kStructureSkeleton) || (type == kStructureBoneCountArray) || (type == kStructureBoneIndexArray) || (type == kStructureBoneWeightArray))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult SkinStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	const Structure *structure = GetFirstSubstructure(kStructureTransform);
	if (structure)
	{
		if (GetLastSubstructure(kStructureTransform) != structure)
		{
			return (kDataExtraneousSubstructure);
		}

		skinTransform = static_cast<const TransformStructure *>(structure)->GetTransform();
		static_cast<OpenGexDataDescription *>(dataDescription)->AdjustTransform(skinTransform);
	}
	else
	{
		skinTransform.SetIdentity();
	}

	structure = GetFirstSubstructure(kStructureSkeleton);
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubstructure(kStructureSkeleton) != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	skeletonStructure = static_cast<const SkeletonStructure *>(structure);

	structure = GetFirstSubstructure(kStructureBoneCountArray);
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubstructure(kStructureBoneCountArray) != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	boneCountArrayStructure = static_cast<const BoneCountArrayStructure *>(structure);

	structure = GetFirstSubstructure(kStructureBoneIndexArray);
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubstructure(kStructureBoneIndexArray) != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	boneIndexArrayStructure = static_cast<const BoneIndexArrayStructure *>(structure);

	structure = GetFirstSubstructure(kStructureBoneWeightArray);
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubstructure(kStructureBoneWeightArray) != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	boneWeightArrayStructure = static_cast<const BoneWeightArrayStructure *>(structure);

	int32 boneIndexCount = boneIndexArrayStructure->GetBoneIndexCount();
	if (boneWeightArrayStructure->GetBoneWeightCount() != boneIndexCount)
	{
		return (kDataOpenGexBoneWeightCountMismatch);
	}

	int32 vertexCount = boneCountArrayStructure->GetVertexCount();
	const uint16 *boneCountArray = boneCountArrayStructure->GetBoneCountArray();

	int32 boneWeightCount = 0;
	for (machine a = 0; a < vertexCount; a++)
	{
		uint32 count = boneCountArray[a];
		boneWeightCount += count;
	}

	if (boneWeightCount != boneIndexCount)
	{
		return (kDataOpenGexBoneWeightCountMismatch);
	}

	// Do application-specific skin processing here.

	return (kDataOkay);
}


MorphStructure::MorphStructure() : OpenGexStructure(kStructureMorph)
{
	// The value of baseFlag indicates whether the base property was actually
	// specified for the structure.

	morphIndex = 0;
	baseFlag = false;
}

MorphStructure::~MorphStructure()
{
}

bool MorphStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "index")
	{
		*type = kDataUInt32;
		*value = &morphIndex;
		return (true);
	}

	if (identifier == "base")
	{
		*type = kDataUInt32;
		*value = &baseIndex;
		baseFlag = true;
		return (true);
	}

	return (false);
}

bool MorphStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kStructureName)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult MorphStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	morphName = nullptr;

	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	morphName = static_cast<const NameStructure *>(structure)->GetName();

	// Do application-specific morph processing here.

	return (kDataOkay);
}


MeshStructure::MeshStructure() : OpenGexStructure(kStructureMesh)
{
	meshLevel = 0;

	skinStructure = nullptr;
}

MeshStructure::~MeshStructure()
{
	indexArrayList.RemoveAllListElements();
}

bool MeshStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "lod")
	{
		*type = kDataUInt32;
		*value = &meshLevel;
		return (true);
	}

	if (identifier == "primitive")
	{
		*type = kDataString;
		*value = &meshPrimitive;
		return (true);
	}

	return (false);
}

bool MeshStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kStructureVertexArray) || (type == kStructureIndexArray) || (type == kStructureSkin))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult MeshStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = Structure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	Structure *structure = GetFirstSubnode();
	while (structure)
	{
		StructureType type = structure->GetStructureType();
		if (type == kStructureVertexArray)
		{
			const VertexArrayStructure *vertexArrayStructure = static_cast<const VertexArrayStructure *>(structure);

			// Process vertex array here.
		}
		else if (type == kStructureIndexArray)
		{
			IndexArrayStructure *indexArrayStructure = static_cast<IndexArrayStructure *>(structure);
			indexArrayList.AppendListElement(indexArrayStructure);

			// Process index array here.
		}
		else if (type == kStructureSkin)
		{
			if (skinStructure)
			{
				return (kDataExtraneousSubstructure);
			}

			skinStructure = static_cast<SkinStructure *>(structure);
		}

		structure = structure->GetNextSubnode();
	}

	// Do application-specific mesh processing here.

	return (kDataOkay);
}


ObjectStructure::ObjectStructure(StructureType type) : OpenGexStructure(type)
{
	SetBaseStructureType(kStructureObject);
}

ObjectStructure::~ObjectStructure()
{
}


GeometryObjectStructure::GeometryObjectStructure() : ObjectStructure(kStructureGeometryObject)
{
	visibleFlag = true;
	shadowFlag = true;
	motionBlurFlag = true;
}

GeometryObjectStructure::~GeometryObjectStructure()
{
	meshMap.RemoveAllMapElements();
}

bool GeometryObjectStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "visible")
	{
		*type = kDataBool;
		*value = &visibleFlag;
		return (true);
	}

	if (identifier == "shadow")
	{
		*type = kDataBool;
		*value = &shadowFlag;
		return (true);
	}

	if (identifier == "motion_blur")
	{
		*type = kDataBool;
		*value = &motionBlurFlag;
		return (true);
	}

	return (false);
}

bool GeometryObjectStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kStructureMesh) || (type == kStructureMorph))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult GeometryObjectStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	int32 meshCount = 0;
	int32 skinCount = 0;

	Structure *structure = GetFirstSubnode();
	while (structure)
	{
		StructureType type = structure->GetStructureType();
		if (type == kStructureMesh)
		{
			MeshStructure *meshStructure = static_cast<MeshStructure *>(structure);
			if (!meshMap.InsertMapElement(meshStructure))
			{
				return (kDataOpenGexDuplicateLod);
			}

			meshCount++;
			skinCount += (meshStructure->GetSkinStructure() != nullptr);
		}
		else if (type == kStructureMorph)
		{
			MorphStructure *morphStructure = static_cast<MorphStructure *>(structure);
			if (!morphMap.InsertMapElement(morphStructure))
			{
				return (kDataOpenGexDuplicateMorph);
			}
		}

		structure = structure->GetNextSubnode();
	}

	if (meshCount == 0)
	{
		return (kDataMissingSubstructure);
	}

	if ((skinCount != 0) && (skinCount != meshCount))
	{
		return (kDataOpenGexMissingLodSkin);
	}

	// Do application-specific object processing here.

	return (kDataOkay);
}


LightObjectStructure::LightObjectStructure() : ObjectStructure(kStructureLightObject)
{
	shadowFlag = true;
}

LightObjectStructure::~LightObjectStructure()
{
}

bool LightObjectStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "type")
	{
		*type = kDataString;
		*value = &typeString;
		return (true);
	}

	if (identifier == "shadow")
	{
		*type = kDataBool;
		*value = &shadowFlag;
		return (true);
	}

	return (false);
}

bool LightObjectStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if ((structure->GetBaseStructureType() == kStructureAttrib) || (structure->GetStructureType() == kStructureAtten))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult LightObjectStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (typeString == "infinite")
	{
		// Prepare to handle infinite light here.
	}
	else if (typeString == "point")
	{
		// Prepare to handle point light here.
	}
	else if (typeString == "spot")
	{
		// Prepare to handle spot light here.
	}
	else
	{
		return (kDataOpenGexUndefinedLightType);
	}

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		StructureType type = structure->GetStructureType();
		if (type == kStructureColor)
		{
			const ColorStructure *colorStructure = static_cast<const ColorStructure *>(structure);
			if (colorStructure->GetAttribString() == "light")
			{
				// Process light color here.
			}
		}
		else if (type == kStructureParam)
		{
			const ParamStructure *paramStructure = static_cast<const ParamStructure *>(structure);
			if (paramStructure->GetAttribString() == "intensity")
			{
				// Process point/spot light intensity here.
			}
			else if (paramStructure->GetAttribString() == "illuminance")
			{
				// Process infinite light illuminance here.
			}
		}
		else if (type == kStructureTexture)
		{
			const TextureStructure *textureStructure = static_cast<const TextureStructure *>(structure);
			if (textureStructure->GetAttribString() == "projection")
			{
				const char *textureName = textureStructure->GetTextureName();

				// Process light texture here.
			}
		}
		else if (type == kStructureAtten)
		{
			const AttenStructure *attenStructure = static_cast<const AttenStructure *>(structure);
			const String<>& attenKind = attenStructure->GetAttenKind();
			const String<>& curveType = attenStructure->GetCurveType();

			if (attenKind == "distance")
			{
				if ((curveType == "linear") || (curveType == "smooth"))
				{
					float beginParam = attenStructure->GetBeginParam();
					float endParam = attenStructure->GetEndParam();

					// Process linear or smooth attenuation here.
				}
				else if (curveType == "inverse")
				{
					float scaleParam = attenStructure->GetScaleParam();
					float linearParam = attenStructure->GetLinearParam();

					// Process inverse attenuation here.
				}
				else if (curveType == "inverse_square")
				{
					float scaleParam = attenStructure->GetScaleParam();
					float quadraticParam = attenStructure->GetQuadraticParam();

					// Process inverse square attenuation here.
				}
				else
				{
					return (kDataOpenGexUndefinedCurve);
				}
			}
			else if (attenKind == "angle")
			{
				float endParam = attenStructure->GetEndParam();

				// Process angular attenutation here.
			}
			else if (attenKind == "cos_angle")
			{
				float endParam = attenStructure->GetEndParam();

				// Process angular attenutation here.
			}
			else
			{
				return (kDataOpenGexUndefinedAtten);
			}
		}

		structure = structure->GetNextSubnode();
	}

	// Do application-specific object processing here.

	return (kDataOkay);
}


CameraObjectStructure::CameraObjectStructure() : ObjectStructure(kStructureCameraObject)
{
}

CameraObjectStructure::~CameraObjectStructure()
{
}

bool CameraObjectStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kStructureParam)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult CameraObjectStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	projectionDistance = 1.0F;
	nearDepth = 0.1F;
	farDepth = 1000.0F;

	const OpenGexDataDescription *openGexDataDescription = static_cast<OpenGexDataDescription *>(dataDescription);
	float distanceScale = openGexDataDescription->GetDistanceScale();
	float angleScale = openGexDataDescription->GetAngleScale();

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == kStructureParam)
		{
			const ParamStructure *paramStructure = static_cast<const ParamStructure *>(structure);
			const String<>& attribString = paramStructure->GetAttribString();
			float param = paramStructure->GetParam();

			if (attribString == "fovy")
			{
				float t = Tan(param * angleScale * 0.5F);
				if (t > Math::min_float)
				{
					projectionDistance = 1.0F / t;
				}
			}
			else if (attribString == "near")
			{
				if (param > Math::min_float)
				{
					nearDepth = param * distanceScale;
				}
			}
			else if (attribString == "far")
			{
				if (param > Math::min_float)
				{
					farDepth = param * distanceScale;
				}
			}
		}

		structure = structure->GetNextSubnode();
	}

	// Do application-specific object processing here.

	return (kDataOkay);
}


AttribStructure::AttribStructure(StructureType type) : OpenGexStructure(type)
{
	SetBaseStructureType(kStructureAttrib);
}

AttribStructure::~AttribStructure()
{
}

bool AttribStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "attrib")
	{
		*type = kDataString;
		*value = &attribString;
		return (true);
	}

	return (false);
}


ParamStructure::ParamStructure() : AttribStructure(kStructureParam)
{
}

ParamStructure::~ParamStructure()
{
}

bool ParamStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		uint32 arraySize = dataStructure->GetArraySize();
		return (arraySize == 0);
	}

	return (AttribStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult ParamStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	if (dataStructure->GetDataElementCount() == 1)
	{
		param = dataStructure->GetDataElement(0);
	}
	else
	{
		return (kDataInvalidDataFormat);
	}

	return (kDataOkay);
}


ColorStructure::ColorStructure() : AttribStructure(kStructureColor)
{
}

ColorStructure::~ColorStructure()
{
}

bool ColorStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		uint32 arraySize = dataStructure->GetArraySize();
		return (arraySize - 3U < 2U);
	}

	return (AttribStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult ColorStructure::ProcessData(DataDescription *dataDescription)
{
	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	int32 arraySize = dataStructure->GetArraySize();
	if (dataStructure->GetDataElementCount() == arraySize)
	{
		const float *data = &dataStructure->GetDataElement(0);

		if (arraySize == 3)
		{
			color.Set(data[0], data[1], data[2], 1.0F);
		}
		else
		{
			color.Set(data[0], data[1], data[2], data[3]);
		}

		static_cast<OpenGexDataDescription *>(dataDescription)->ConvertColor(color.GetColorRGB());
	}
	else
	{
		return (kDataInvalidDataFormat);
	}

	return (kDataOkay);
}


SpectrumStructure::SpectrumStructure() : AttribStructure(kStructureSpectrum)
{
	wavelengthRange.Set(0, 0);
}

SpectrumStructure::~SpectrumStructure()
{
}

bool SpectrumStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	return (AttribStructure::ValidateProperty(dataDescription, identifier, type, value));
}

bool SpectrumStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
		return (dataStructure->GetArraySize() == 0);
	}

	return (AttribStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult SpectrumStructure::ProcessData(DataDescription *dataDescription)
{
	// Process spectrum here.

	return (kDataOkay);
}


TextureStructure::TextureStructure() : AttribStructure(kStructureTexture)
{
	texcoordIndex = 0;
	textureSwizzle = "i";
	textureAddress[0] = "repeat";
	textureAddress[1] = "repeat";
	textureAddress[2] = "repeat";
	textureBorder = "zero";

	texcoordTransform.SetIdentity();
}

TextureStructure::~TextureStructure()
{
}

bool TextureStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "texcoord")
	{
		*type = kDataUInt32;
		*value = &texcoordIndex;
		return (true);
	}

	if (identifier == "swizzle")
	{
		*type = kDataString;
		*value = &textureSwizzle;
		return (true);
	}

	if (identifier == "x_address")
	{
		*type = kDataString;
		*value = &textureAddress[0];
		return (true);
	}

	if (identifier == "y_address")
	{
		*type = kDataString;
		*value = &textureAddress[1];
		return (true);
	}

	if (identifier == "z_address")
	{
		*type = kDataString;
		*value = &textureAddress[2];
		return (true);
	}

	if (identifier == "border")
	{
		*type = kDataString;
		*value = &textureBorder;
		return (true);
	}

	return (AttribStructure::ValidateProperty(dataDescription, identifier, type, value));
}

bool TextureStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kDataString) || (type == kStructureAnimation) || (structure->GetBaseStructureType() == kStructureMatrix))
	{
		return (true);
	}

	return (AttribStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult TextureStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = AttribStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	bool nameFlag = false;

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == kDataString)
		{
			if (!nameFlag)
			{
				nameFlag = true;

				const DataStructure<StringDataType> *dataStructure = static_cast<const DataStructure<StringDataType> *>(structure);
				if (dataStructure->GetDataElementCount() == 1)
				{
					textureName = dataStructure->GetDataElement(0);
				}
				else
				{
					return (kDataInvalidDataFormat);
				}
			}
			else
			{
				return (kDataExtraneousSubstructure);
			}
		}
		else if (structure->GetBaseStructureType() == kStructureMatrix)
		{
			const MatrixStructure *matrixStructure = static_cast<const MatrixStructure *>(structure);
			texcoordTransform = texcoordTransform * matrixStructure->GetMatrix();
		}

		structure = structure->GetNextSubnode();
	}

	if (!nameFlag)
	{
		return (kDataMissingSubstructure);
	}

	return (kDataOkay);
}


AttenStructure::AttenStructure() :
		OpenGexStructure(kStructureAtten),
		attenKind("distance"),
		curveType("linear")
{
	beginParam = 0.0F;
	endParam = 1.0F;

	scaleParam = 1.0F;
	offsetParam = 0.0F;

	constantParam = 0.0F;
	linearParam = 0.0F;
	quadraticParam = 1.0F;

	powerParam = 1.0F;
}

AttenStructure::~AttenStructure()
{
}

bool AttenStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "kind")
	{
		*type = kDataString;
		*value = &attenKind;
		return (true);
	}

	if (identifier == "curve")
	{
		*type = kDataString;
		*value = &curveType;
		return (true);
	}

	return (false);
}

bool AttenStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kStructureParam)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult AttenStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (curveType == "inverse")
	{
		linearParam = 1.0F;
	}

	const OpenGexDataDescription *openGexDataDescription = static_cast<OpenGexDataDescription *>(dataDescription);
	float distanceScale = openGexDataDescription->GetDistanceScale();
	float angleScale = openGexDataDescription->GetAngleScale();

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == kStructureParam)
		{
			const ParamStructure *paramStructure = static_cast<const ParamStructure *>(structure);
			const String<>& attribString = paramStructure->GetAttribString();

			if (attribString == "begin")
			{
				beginParam = paramStructure->GetParam();

				if (attenKind == "distance")
				{
					beginParam *= distanceScale;
				}
				else if (attenKind == "angle")
				{
					beginParam *= angleScale;
				}
			}
			else if (attribString == "end")
			{
				endParam = paramStructure->GetParam();

				if (attenKind == "distance")
				{
					endParam *= distanceScale;
				}
				else if (attenKind == "angle")
				{
					endParam *= angleScale;
				}
			}
			else if (attribString == "scale")
			{
				scaleParam = paramStructure->GetParam();

				if (attenKind == "distance")
				{
					scaleParam *= distanceScale;
				}
				else if (attenKind == "angle")
				{
					scaleParam *= angleScale;
				}
			}
			else if (attribString == "offset")
			{
				offsetParam = paramStructure->GetParam();
			}
			else if (attribString == "constant")
			{
				constantParam = paramStructure->GetParam();
			}
			else if (attribString == "linear")
			{
				linearParam = paramStructure->GetParam();
			}
			else if (attribString == "quadratic")
			{
				quadraticParam = paramStructure->GetParam();
			}
			else if (attribString == "power")
			{
				powerParam = paramStructure->GetParam();
			}
		}

		structure = structure->GetNextSubnode();
	}

	return (kDataOkay);
}


MaterialStructure::MaterialStructure() : OpenGexStructure(kStructureMaterial)
{
	twoSidedFlag = false;
	materialName = nullptr;
}

MaterialStructure::~MaterialStructure()
{
}

bool MaterialStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "two_sided")
	{
		*type = kDataBool;
		*value = &twoSidedFlag;
		return (true);
	}

	return (false);
}

bool MaterialStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if ((structure->GetBaseStructureType() == kStructureAttrib) || (structure->GetStructureType() == kStructureName))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult MaterialStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	const Structure *structure = GetFirstSubstructure(kStructureName);
	if (structure)
	{
		if (GetLastSubstructure(kStructureName) != structure)
		{
			return (kDataExtraneousSubstructure);
		}
	}

	// Do application-specific material processing here.

	return (kDataOkay);
}


KeyStructure::KeyStructure() :
		OpenGexStructure(kStructureKey),
		keyKind("value")
{
}

KeyStructure::~KeyStructure()
{
}

bool KeyStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "kind")
	{
		*type = kDataString;
		*value = &keyKind;
		return (true);
	}

	return (false);
}

bool KeyStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kDataFloat)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult KeyStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	const Structure *structure = GetFirstSubnode();
	if (!structure)
	{
		return (kDataMissingSubstructure);
	}

	if (GetLastSubnode() != structure)
	{
		return (kDataExtraneousSubstructure);
	}

	const DataStructure<FloatDataType> *dataStructure = static_cast<const DataStructure<FloatDataType> *>(structure);
	if (dataStructure->GetDataElementCount() == 0)
	{
		return (kDataOpenGexEmptyKeyStructure);
	}

	if ((keyKind == "value") || (keyKind == "-control") || (keyKind == "+control"))
	{
		scalarFlag = false;
	}
	else if ((keyKind == "tension") || (keyKind == "continuity") || (keyKind == "bias"))
	{
		scalarFlag = true;

		if (dataStructure->GetArraySize() != 0)
		{
			return (kDataInvalidDataFormat);
		}
	}
	else
	{
		return (kDataOpenGexInvalidKeyKind);
	}

	return (kDataOkay);
}


CurveStructure::CurveStructure(StructureType type) :
		OpenGexStructure(type),
		curveType("linear")
{
	SetBaseStructureType(kStructureCurve);
}

CurveStructure::~CurveStructure()
{
}

bool CurveStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "curve")
	{
		*type = kDataString;
		*value = &curveType;
		return (true);
	}

	return (false);
}

bool CurveStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kStructureKey)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult CurveStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	keyValueStructure = nullptr;
	keyControlStructure[0] = nullptr;
	keyControlStructure[1] = nullptr;
	keyTensionStructure = nullptr;
	keyContinuityStructure = nullptr;
	keyBiasStructure = nullptr;

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == kStructureKey)
		{
			const KeyStructure *keyStructure = static_cast<const KeyStructure *>(structure);
			const String<>& keyKind = keyStructure->GetKeyKind();

			if (keyKind == "value")
			{
				if (!keyValueStructure)
				{
					keyValueStructure = keyStructure;
				}
				else
				{
					return (kDataExtraneousSubstructure);
				}
			}
			else if (keyKind == "-control")
			{
				if (curveType != "bezier")
				{
					return (kDataOpenGexInvalidKeyKind);
				}

				if (!keyControlStructure[0])
				{
					keyControlStructure[0] = keyStructure;
				}
				else
				{
					return (kDataExtraneousSubstructure);
				}
			}
			else if (keyKind == "+control")
			{
				if (curveType != "bezier")
				{
					return (kDataOpenGexInvalidKeyKind);
				}

				if (!keyControlStructure[1])
				{
					keyControlStructure[1] = keyStructure;
				}
				else
				{
					return (kDataExtraneousSubstructure);
				}
			}
			else if (keyKind == "tension")
			{
				if (curveType != "tcb")
				{
					return (kDataOpenGexInvalidKeyKind);
				}

				if (!keyTensionStructure)
				{
					keyTensionStructure = keyStructure;
				}
				else
				{
					return (kDataExtraneousSubstructure);
				}
			}
			else if (keyKind == "continuity")
			{
				if (curveType != "tcb")
				{
					return (kDataOpenGexInvalidKeyKind);
				}

				if (!keyContinuityStructure)
				{
					keyContinuityStructure = keyStructure;
				}
				else
				{
					return (kDataExtraneousSubstructure);
				}
			}
			else if (keyKind == "bias")
			{
				if (curveType != "tcb")
				{
					return (kDataOpenGexInvalidKeyKind);
				}

				if (!keyBiasStructure)
				{
					keyBiasStructure = keyStructure;
				}
				else
				{
					return (kDataExtraneousSubstructure);
				}
			}
		}

		structure = structure->GetNextSubnode();
	}

	if (!keyValueStructure)
	{
		return (kDataMissingSubstructure);
	}

	if (curveType == "bezier")
	{
		if ((!keyControlStructure[0]) || (!keyControlStructure[1]))
		{
			return (kDataMissingSubstructure);
		}
	}
	else if (curveType == "tcb")
	{
		if ((!keyTensionStructure) || (!keyContinuityStructure) || (!keyBiasStructure))
		{
			return (kDataMissingSubstructure);
		}
	}

	return (kDataOkay);
}


TimeStructure::TimeStructure() : CurveStructure(kStructureTime)
{
}

TimeStructure::~TimeStructure()
{
}

DataResult TimeStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = CurveStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	const String<>& curveType = GetCurveType();
	if ((curveType != "linear") && (curveType != "bezier"))
	{
		return (kDataOpenGexInvalidCurveType);
	}

	int32 elementCount = 0;

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == kStructureKey)
		{
			const KeyStructure *keyStructure = static_cast<const KeyStructure *>(structure);
			const DataStructure<FloatDataType> *dataStructure = static_cast<DataStructure<FloatDataType> *>(keyStructure->GetFirstSubnode());
			if (dataStructure->GetArraySize() != 0)
			{
				return (kDataInvalidDataFormat);
			}

			int32 count = dataStructure->GetDataElementCount();
			if (elementCount == 0)
			{
				elementCount = count;
			}
			else if (count != elementCount)
			{
				return (kDataOpenGexKeyCountMismatch);
			}
		}

		structure = structure->GetNextSubnode();
	}

	keyDataElementCount = elementCount;
	return (kDataOkay);
}

int32 TimeStructure::CalculateInterpolationParameter(float time, float *param) const
{
	const DataStructure<FloatDataType> *valueStructure = static_cast<DataStructure<FloatDataType> *>(GetKeyValueStructure()->GetFirstSubnode());
	const float *value = &valueStructure->GetDataElement(0);

	int32 count = keyDataElementCount;
	int32 index = 0;

	for (; index < count; index++)
	{
		if (time < value[index])
		{
			break;
		}
	}

	if ((index > 0) && (index < count))
	{
		float t0 = value[index - 1];
		float t3 = value[index];

		float u = 0.0F;
		float dt = t3 - t0;
		if (dt > Math::min_float)
		{
			u = (time - t0) / dt;
		}

		if (GetCurveType() == "bezier")
		{
			float t1 = static_cast<DataStructure<FloatDataType> *>(GetKeyControlStructure(1)->GetFirstSubnode())->GetDataElement(index - 1);
			float t2 = static_cast<DataStructure<FloatDataType> *>(GetKeyControlStructure(0)->GetFirstSubnode())->GetDataElement(index);

			float a0 = dt + (t1 - t2) * 3.0F;
			float a1 = a0 * 3.0F;
			float b0 = 3.0F * (t0 - t1 * 2.0F + t2);
			float b1 = b0 * 2.0F;
			float c = (t1 - t0) * 3.0F;
			float d = t0 - time;

			for (machine k = 0; k < 3; k++)
			{
				u = Saturate(u - ((((a0 * u) + b0) * u + c) * u + d) / (((a1 * u) + b1) * u + c));
			}
		}

		*param = u;
	}
	else
	{
		*param = 0.0F;
	}

	return (index - 1);
}


ValueStructure::ValueStructure() : CurveStructure(kStructureValue)
{
}

ValueStructure::~ValueStructure()
{
}

DataResult ValueStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = CurveStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	const String<>& curveType = GetCurveType();
	if ((curveType != "constant") && (curveType != "linear") && (curveType != "bezier") && (curveType != "tcb"))
	{
		return (kDataOpenGexInvalidCurveType);
	}

	const AnimatableStructure *targetStructure = static_cast<TrackStructure *>(GetSuperNode())->GetTargetStructure();
	const Structure *targetDataStructure = targetStructure->GetFirstSubnode();
	if ((targetDataStructure) && (targetDataStructure->GetStructureType() == kDataFloat))
	{
		uint32 targetArraySize = static_cast<const PrimitiveStructure *>(targetDataStructure)->GetArraySize();
		int32 elementCount = 0;

		const Structure *structure = GetFirstSubnode();
		while (structure)
		{
			if (structure->GetStructureType() == kStructureKey)
			{
				const KeyStructure *keyStructure = static_cast<const KeyStructure *>(structure);
				const DataStructure<FloatDataType> *dataStructure = static_cast<DataStructure<FloatDataType> *>(keyStructure->GetFirstSubnode());
				uint32 arraySize = dataStructure->GetArraySize();

				if ((!keyStructure->GetScalarFlag()) && (arraySize != targetArraySize))
				{
					return (kDataInvalidDataFormat);
				}

				int32 count = dataStructure->GetDataElementCount() / Max(arraySize, 1);
				if (elementCount == 0)
				{
					elementCount = count;
				}
				else if (count != elementCount)
				{
					return (kDataOpenGexKeyCountMismatch);
				}
			}

			structure = structure->GetNextSubnode();
		}

		keyDataElementCount = elementCount;
	}

	return (kDataOkay);
}

void ValueStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, int32 index, float param, AnimatableStructure *target) const
{
	float	data[16];

	const String<>& curveType = GetCurveType();
	const float *value = &static_cast<DataStructure<FloatDataType> *>(GetKeyValueStructure()->GetFirstSubnode())->GetDataElement(0);

	int32 count = keyDataElementCount;
	int32 arraySize = Max(static_cast<PrimitiveStructure *>(target->GetFirstSubnode())->GetArraySize(), 1);

	if (index < 0)
	{
		target->UpdateAnimation(dataDescription, value);
	}
	else if (index >= count - 1)
	{
		target->UpdateAnimation(dataDescription, value + arraySize * (count - 1));
	}
	else
	{
		const float *p1 = value + arraySize * index;

		if (curveType == "constant")
		{
			for (machine k = 0; k < arraySize; k++)
			{
				data[k] = p1[k];
			}
		}
		else
		{
			const float *p2 = p1 + arraySize;
			const float u = 1.0F - param;

			if (curveType == "linear")
			{
				for (machine k = 0; k < arraySize; k++)
				{
					data[k] = p1[k] * u + p2[k] * param;
				}
			}
			else if (curveType == "bezier")
			{
				const float *c1 = &static_cast<DataStructure<FloatDataType> *>(GetKeyControlStructure(1)->GetFirstSubnode())->GetDataElement(arraySize * index);
				const float *c2 = &static_cast<DataStructure<FloatDataType> *>(GetKeyControlStructure(0)->GetFirstSubnode())->GetDataElement(arraySize * (index + 1));

				float u2 = u * u;
				float u3 = u2 * u;
				float v2 = param * param;
				float v3 = v2 * param;
				float f1 = u2 * param * 3.0F;
				float f2 = u * v2 * 3.0F;

				for (machine k = 0; k < arraySize; k++)
				{
					data[k] = p1[k] * u3 + c1[k] * f1 + c2[k] * f2 + p2[k] * v3;
				}
			}
			else
			{
				const float *p0 = value + arraySize * MaxZero(index - 1);
				const float *p3 = value + arraySize * Min(index + 2, count - 1);

				const float *tension = &static_cast<DataStructure<FloatDataType> *>(GetKeyTensionStructure()->GetFirstSubnode())->GetDataElement(0);
				const float *continuity = &static_cast<DataStructure<FloatDataType> *>(GetKeyContinuityStructure()->GetFirstSubnode())->GetDataElement(0);
				const float *bias = &static_cast<DataStructure<FloatDataType> *>(GetKeyBiasStructure()->GetFirstSubnode())->GetDataElement(0);

				float m1 = (1.0F - tension[index]) * (1.0F + continuity[index]) * (1.0F + bias[index]) * 0.5F;
				float n1 = (1.0F - tension[index]) * (1.0F - continuity[index]) * (1.0F - bias[index]) * 0.5F;
				float m2 = (1.0F - tension[index + 1]) * (1.0F - continuity[index + 1]) * (1.0F + bias[index + 1]) * 0.5F;
				float n2 = (1.0F - tension[index + 1]) * (1.0F + continuity[index + 1]) * (1.0F - bias[index + 1]) * 0.5F;

				float u2 = u * u;
				float v2 = param * param;
				float v3 = v2 * param;
				float f1 = 1.0F - v2 * 3.0F + v3 * 2.0F;
				float f2 = v2 * (3.0F - param * 2.0F);
				float f3 = param * u2;
				float f4 = u * v2;

				for (machine k = 0; k < arraySize; k++)
				{
					float t1 = (p1[k] - p0[k]) * m1 + (p2[k] - p1[k]) * n1;
					float t2 = (p2[k] - p1[k]) * m2 + (p3[k] - p2[k]) * n2;
					data[k] = p1[k] * f1 + p2[k] * f2 + t1 * f3 - t2 * f4;
				}
			}
		}

		target->UpdateAnimation(dataDescription, data);
	}
}


TrackStructure::TrackStructure() : OpenGexStructure(kStructureTrack)
{
	targetStructure = nullptr;
}

TrackStructure::~TrackStructure()
{
}

bool TrackStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "target")
	{
		*type = kDataRef;
		*value = &targetRef;
		return (true);
	}

	return (false);
}

bool TrackStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetBaseStructureType() == kStructureCurve)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult TrackStructure::ProcessData(DataDescription *dataDescription)
{
	if (targetRef.GetGlobalRefFlag())
	{
		return (kDataOpenGexTargetRefNotLocal);
	}

	Structure *target = GetSuperNode()->GetSuperNode()->FindStructure(targetRef);
	if (!target)
	{
		return (kDataBrokenRef);
	}

	if ((target->GetBaseStructureType() != kStructureMatrix) && (target->GetStructureType() != kStructureMorphWeight))
	{
		return (kDataOpenGexInvalidTargetStruct);
	}

	targetStructure = static_cast<AnimatableStructure *>(target);

	timeStructure = nullptr;
	valueStructure = nullptr;

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		StructureType type = structure->GetStructureType();
		if (type == kStructureTime)
		{
			if (!timeStructure)
			{
				timeStructure = static_cast<const TimeStructure *>(structure);
			}
			else
			{
				return (kDataExtraneousSubstructure);
			}
		}
		else if (type == kStructureValue)
		{
			if (!valueStructure)
			{
				valueStructure = static_cast<const ValueStructure *>(structure);
			}
			else
			{
				return (kDataExtraneousSubstructure);
			}
		}

		structure = structure->GetNextSubnode();
	}

	if ((!timeStructure) || (!valueStructure))
	{
		return (kDataMissingSubstructure);
	}

	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (timeStructure->GetKeyDataElementCount() != valueStructure->GetKeyDataElementCount())
	{
		return (kDataOpenGexKeyCountMismatch);
	}

	static_cast<AnimationStructure *>(GetSuperNode())->AddTrack(this);
	return (kDataOkay);
}

void TrackStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, float time) const
{
	float	param;

	int32 index = timeStructure->CalculateInterpolationParameter(time, &param);
	valueStructure->UpdateAnimation(dataDescription, index, param, targetStructure);
}


AnimationStructure::AnimationStructure() : OpenGexStructure(kStructureAnimation)
{
	clipIndex = 0;
	beginFlag = false;
	endFlag = false;
}

AnimationStructure::~AnimationStructure()
{
	trackList.RemoveAllListElements();
}

bool AnimationStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "clip")
	{
		*type = kDataUInt32;
		*value = &clipIndex;
		return (true);
	}

	if (identifier == "begin")
	{
		beginFlag = true;
		*type = kDataFloat;
		*value = &beginTime;
		return (true);
	}

	if (identifier == "end")
	{
		endFlag = true;
		*type = kDataFloat;
		*value = &endTime;
		return (true);
	}

	return (false);
}

bool AnimationStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	if (structure->GetStructureType() == kStructureTrack)
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult AnimationStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (trackList.Empty())
	{
		return (kDataMissingSubstructure);
	}

	static_cast<OpenGexDataDescription *>(dataDescription)->AddAnimation(this);
	return (kDataOkay);
}

Range<float> AnimationStructure::GetAnimationTimeRange(void) const
{
	float min = Math::infinity;
	float max = 0.0F;

	const TrackStructure *trackStructure = trackList.GetFirstListElement();
	while (trackStructure)
	{
		const KeyStructure *keyStructure = trackStructure->GetTimeStructure()->GetKeyValueStructure();
		const DataStructure<FloatDataType> *dataStructure = static_cast<DataStructure<FloatDataType> *>(keyStructure->GetFirstSubnode());

		min = Fmin(min, dataStructure->GetDataElement(0));
		max = Fmax(max, dataStructure->GetDataElement(dataStructure->GetDataElementCount() - 1));

		trackStructure = trackStructure->GetNextListElement();
	}

	if (beginFlag)
	{
		min = beginTime;
	}

	if (endFlag)
	{
		max = endTime;
	}

	return (Range<float>(min, Fmax(min, max)));
}

void AnimationStructure::UpdateAnimation(const OpenGexDataDescription *dataDescription, float time) const
{
	if (beginFlag)
	{
		time = Fmax(time, beginTime);
	}

	if (endFlag)
	{
		time = Fmin(time, endTime);
	}

	const TrackStructure *trackStructure = trackList.GetFirstListElement();
	while (trackStructure)
	{
		trackStructure->UpdateAnimation(dataDescription, time);
		trackStructure = trackStructure->GetNextListElement();
	}
}


ClipStructure::ClipStructure() : OpenGexStructure(kStructureClip)
{
	clipIndex = 0;
}

ClipStructure::~ClipStructure()
{
}

bool ClipStructure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	if (identifier == "index")
	{
		*type = kDataUInt32;
		*value = &clipIndex;
		return (true);
	}

	return (false);
}

bool ClipStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	StructureType type = structure->GetStructureType();
	if ((type == kStructureName) || (type == kStructureParam))
	{
		return (true);
	}

	return (OpenGexStructure::ValidateSubstructure(dataDescription, structure));
}

DataResult ClipStructure::ProcessData(DataDescription *dataDescription)
{
	DataResult result = OpenGexStructure::ProcessData(dataDescription);
	if (result != kDataOkay)
	{
		return (result);
	}

	frameRate = 0.0F;
	clipName = nullptr;

	const Structure *structure = GetFirstSubnode();
	while (structure)
	{
		StructureType type = structure->GetStructureType();
		if (type == kStructureName)
		{
			if (clipName)
			{
				return (kDataExtraneousSubstructure);
			}

			clipName = static_cast<const NameStructure *>(structure)->GetName();
		}
		else if (type == kStructureParam)
		{
			const ParamStructure *paramStructure = static_cast<const ParamStructure *>(structure);
			if (paramStructure->GetAttribString() == "rate")
			{
				frameRate = paramStructure->GetParam();
			}
		}

		structure = structure->GetNextSubnode();
	}

	return (kDataOkay);
}


OpenGexDataDescription::OpenGexDataDescription()
{
	distanceScale = 1.0F;
	angleScale = 1.0F;
	timeScale = 1.0F;
	upDirection = "z";
	forwardDirection = "x";
	redChromaticity.Set(0.64F, 0.33F);
	greenChromaticity.Set(0.3F, 0.6F);
	blueChromaticity.Set(0.15F, 0.06F);
	whiteChromaticity.Set(0.3127F, 0.329F);
}

OpenGexDataDescription::~OpenGexDataDescription()
{
	animationList.RemoveAllListElements();
}

Structure *OpenGexDataDescription::CreateStructure(const String<>& identifier) const
{
	if (identifier == "Metric")
	{
		return (new MetricStructure);
	}

	if (identifier == "Name")
	{
		return (new NameStructure);
	}

	if (identifier == "ObjectRef")
	{
		return (new ObjectRefStructure);
	}

	if (identifier == "MaterialRef")
	{
		return (new MaterialRefStructure);
	}

	if (identifier == "Transform")
	{
		return (new TransformStructure);
	}

	if (identifier == "Translation")
	{
		return (new TranslationStructure);
	}

	if (identifier == "Rotation")
	{
		return (new RotationStructure);
	}

	if (identifier == "Scale")
	{
		return (new ScaleStructure);
	}

	if (identifier == "MorphWeight")
	{
		return (new MorphWeightStructure);
	}

	if (identifier == "Node")
	{
		return (new NodeStructure);
	}

	if (identifier == "BoneNode")
	{
		return (new BoneNodeStructure);
	}

	if (identifier == "GeometryNode")
	{
		return (new GeometryNodeStructure);
	}

	if (identifier == "LightNode")
	{
		return (new LightNodeStructure);
	}

	if (identifier == "CameraNode")
	{
		return (new CameraNodeStructure);
	}

	if (identifier == "VertexArray")
	{
		return (new VertexArrayStructure);
	}

	if (identifier == "IndexArray")
	{
		return (new IndexArrayStructure);
	}

	if (identifier == "BoneRefArray")
	{
		return (new BoneRefArrayStructure);
	}

	if (identifier == "BoneCountArray")
	{
		return (new BoneCountArrayStructure);
	}

	if (identifier == "BoneIndexArray")
	{
		return (new BoneIndexArrayStructure);
	}

	if (identifier == "BoneWeightArray")
	{
		return (new BoneWeightArrayStructure);
	}

	if (identifier == "Skeleton")
	{
		return (new SkeletonStructure);
	}

	if (identifier == "Skin")
	{
		return (new SkinStructure);
	}

	if (identifier == "Morph")
	{
		return (new MorphStructure);
	}

	if (identifier == "Mesh")
	{
		return (new MeshStructure);
	}

	if (identifier == "GeometryObject")
	{
		return (new GeometryObjectStructure);
	}

	if (identifier == "LightObject")
	{
		return (new LightObjectStructure);
	}

	if (identifier == "CameraObject")
	{
		return (new CameraObjectStructure);
	}

	if (identifier == "Param")
	{
		return (new ParamStructure);
	}

	if (identifier == "Color")
	{
		return (new ColorStructure);
	}

	if (identifier == "Spectrum")
	{
		return (new SpectrumStructure);
	}

	if (identifier == "Texture")
	{
		return (new TextureStructure);
	}

	if (identifier == "Atten")
	{
		return (new AttenStructure);
	}

	if (identifier == "Material")
	{
		return (new MaterialStructure);
	}

	if (identifier == "Key")
	{
		return (new KeyStructure);
	}

	if (identifier == "Time")
	{
		return (new TimeStructure);
	}

	if (identifier == "Value")
	{
		return (new ValueStructure);
	}

	if (identifier == "Track")
	{
		return (new TrackStructure);
	}

	if (identifier == "Animation")
	{
		return (new AnimationStructure);
	}

	if (identifier == "Clip")
	{
		return (new ClipStructure);
	}

	return (nullptr);
}

bool OpenGexDataDescription::ValidateTopLevelStructure(const Structure *structure) const
{
	StructureType type = structure->GetBaseStructureType();
	if ((type == kStructureNode) || (type == kStructureObject))
	{
		return (true);
	}

	type = structure->GetStructureType();
	return ((type == kStructureMetric) || (type == kStructureMaterial) || (type == kStructureClip));
}

DataResult OpenGexDataDescription::ProcessData(void)
{
	colorInitFlag = false;

	DataResult result = DataDescription::ProcessData();
	if (result == kDataOkay)
	{
		Structure *structure = GetRootStructure()->GetFirstSubnode();
		while (structure)
		{
			if (structure->GetBaseStructureType() == kStructureNode)
			{
				static_cast<NodeStructure *>(structure)->UpdateNodeTransforms(this);
			}

			structure = structure->GetNextSubnode();
		}
	}

	return (result);
}

void OpenGexDataDescription::AdjustTransform(Transform4D& transform) const
{
	transform.SetTranslation(transform.GetTranslation() * distanceScale);

	if (upDirection[0] == 'y')
	{
		transform.Set( transform(0,0), -transform(0,2),  transform(0,1),  transform(0,3),
					  -transform(2,0),  transform(2,2), -transform(2,1), -transform(2,3),
					   transform(1,0), -transform(1,2),  transform(1,1),  transform(1,3));
	}
}

void OpenGexDataDescription::ConvertColor(ColorRGB& color)
{
	if (!colorInitFlag)
	{
		colorInitFlag = true;

		float xr = redChromaticity.x;
		float xg = greenChromaticity.x;
		float xb = blueChromaticity.x;
		float xw = whiteChromaticity.x;

		float zr = 1.0F - xr - redChromaticity.y;
		float zg = 1.0F - xg - greenChromaticity.y;
		float zb = 1.0F - xb - blueChromaticity.y;
		float zw = 1.0F - xw - whiteChromaticity.y;

		float iyr = 1.0F / redChromaticity.y;
		float iyg = 1.0F / greenChromaticity.y;
		float iyb = 1.0F / blueChromaticity.y;
		float iyw = 1.0F / whiteChromaticity.y;

		Matrix3D m(xr * iyr, xg * iyg, xb * iyb, 1.0F, 1.0F, 1.0F, zr * iyr, zg * iyg, zb * iyb);

		Vector3D lum = Inverse(m) * Vector3D(xw * iyw, 1.0F, zw * iyw);
		m[0] *= lum.x;
		m[1] *= lum.y;
		m[2] *= lum.z;

		colorMatrix = Matrix3D(3.24097F, -1.537383F, -0.498611F, -0.969244F, 1.875968F, 0.041555F, 0.05563F, -0.203977F, 1.056972F) * m;
	}

	Vector3D& v = reinterpret_cast<Vector3D&>(color);
	v = colorMatrix * v;
}

Range<float> OpenGexDataDescription::GetAnimationTimeRange(int32 clip) const
{
	Range<float>	timeRange;

	bool animationFlag = false;

	const AnimationStructure *animationStructure = animationList.GetFirstListElement();
	while (animationStructure)
	{
		if (animationStructure->GetClipIndex() == clip)
		{
			if (animationFlag)
			{
				Range<float> range = animationStructure->GetAnimationTimeRange();
				timeRange.min = Fmin(timeRange.min, range.min);
				timeRange.max = Fmax(timeRange.max, range.max);
			}
			else
			{
				animationFlag = true;
				timeRange = animationStructure->GetAnimationTimeRange();
			}
		}

		animationStructure = animationStructure->GetNextListElement();
	}

	if (animationFlag)
	{
		timeRange.min *= timeScale;
		timeRange.max *= timeScale;
		return (timeRange);
	}

	return (Range<float>(0.0F, 0.0F));
}

void OpenGexDataDescription::UpdateAnimation(int32 clip, float time) const
{
	time /= timeScale;

	const AnimationStructure *animationStructure = animationList.GetFirstListElement();
	while (animationStructure)
	{
		if (animationStructure->GetClipIndex() == clip)
		{
			animationStructure->UpdateAnimation(this, time);
		}

		animationStructure = animationStructure->GetNextListElement();
	}

	Structure *structure = GetRootStructure()->GetFirstSubnode();
	while (structure)
	{
		if (structure->GetBaseStructureType() == kStructureNode)
		{
			static_cast<NodeStructure *>(structure)->UpdateNodeTransforms(this);
		}

		structure = structure->GetNextSubnode();
	}
}
