import lldb

class Simd128Printer:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj

    def update(self):
        self.v0 = self.valobj.GetChildAtIndex(0).GetValueAsUnsigned()
        self.v1 = self.valobj.GetChildAtIndex(1).GetValueAsUnsigned()

    def num_children(self):
        return 8

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        if index == 0:
            return self.valobj.CreateValueFromExpression('u8x16', 'uint16_t x[16] = {' + str(self.v0 & 0xFF) + ',' + str((self.v0 >> 8) & 0xFF) + ',' + str((self.v0 >> 16) & 0xFF) + ',' + str((self.v0 >> 24) & 0xFF) + ',' + str((self.v0 >> 32) & 0xFF) + ',' + str((self.v0 >> 40) & 0xFF) + ',' + str((self.v0 >> 48) & 0xFF) + ',' + str(self.v0 >> 56) + ',' + str(self.v1 & 0xFF) + ',' + str((self.v1 >> 8) & 0xFF) + ',' + str((self.v1 >> 16) & 0xFF) + ',' + str((self.v1 >> 24) & 0xFF) + ',' + str((self.v1 >> 32) & 0xFF) + ',' + str((self.v1 >> 40) & 0xFF) + ',' + str((self.v1 >> 48) & 0xFF) + ',' + str(self.v1 >> 56) + '}; x')
        elif index == 1:
            return self.valobj.CreateValueFromExpression('i8x16', 'int16_t x[16] = {(int8_t)' + str(self.v0 & 0xFF) + ',(int8_t)' + str((self.v0 >> 8) & 0xFF) + ',(int8_t)' + str((self.v0 >> 16) & 0xFF) + ',(int8_t)' + str((self.v0 >> 24) & 0xFF) + ',(int8_t)' + str((self.v0 >> 32) & 0xFF) + ',(int8_t)' + str((self.v0 >> 40) & 0xFF) + ',(int8_t)' + str((self.v0 >> 48) & 0xFF) + ',(int8_t)' + str(self.v0 >> 56) + ',(int8_t)' + str(self.v1 & 0xFF) + ',(int8_t)' + str((self.v1 >> 8) & 0xFF) + ',(int8_t)' + str((self.v1 >> 16) & 0xFF) + ',(int8_t)' + str((self.v1 >> 24) & 0xFF) + ',(int8_t)' + str((self.v1 >> 32) & 0xFF) + ',(int8_t)' + str((self.v1 >> 40) & 0xFF) + ',(int8_t)' + str((self.v1 >> 48) & 0xFF) + ',(int8_t)' + str(self.v1 >> 56) + '}; x')
        elif index == 2:
            return self.valobj.CreateValueFromExpression('u16x8', 'uint16_t x[8] = {' + str(self.v0 & 0xFFFF) + ',' + str((self.v0 >> 16) & 0xFFFF) + ',' + str((self.v0 >> 32) & 0xFFFF) + ',' + str(self.v0 >> 48) + ',' + str(self.v1 & 0xFFFF) + ',' + str((self.v1 >> 16) & 0xFFFF) + ',' + str((self.v1 >> 32) & 0xFFFF) + ',' + str(self.v1 >> 48) + '}; x')
        elif index == 3:
            return self.valobj.CreateValueFromExpression('i16x8', 'int16_t x[8] = {(int16_t)' + str(self.v0 & 0xFFFF) + ',(int16_t)' + str((self.v0 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v0 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v0 >> 48) + ',(int16_t)' + str(self.v1 & 0xFFFF) + ',(int16_t)' + str((self.v1 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v1 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v1 >> 48) + '}; x')
        elif index == 4:
            return self.valobj.CreateValueFromExpression('u32x4', 'uint32_t x[4] = {' + str(self.v0 & 0xFFFFFFFF) + ',' + str(self.v0 >> 32) + ',' + str(self.v1 & 0xFFFFFFFF) + ',' + str(self.v1 >> 32) + '}; x')
        elif index == 5:
            return self.valobj.CreateValueFromExpression('i32x4', 'int32_t x[4] = {(int32_t)' + str(self.v0 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v0 >> 32) + ',(int32_t)' + str(self.v1 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v1 >> 32) + '}; x')
        elif index == 6:
            return self.valobj.CreateValueFromExpression('u64x2', 'uint64_t x[2] = {' + str(self.v0) + ',' + str(self.v1) + '}; x')
        elif index == 7:
            return self.valobj.CreateValueFromExpression('i64x2', 'int64_t x[2] = {(int64_t)' + str(self.v0) + ',(int64_t)' + str(self.v1) + '}; x')
        else:
            return None

class Simd256Printer:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj

    def update(self):
        self.v0 = self.valobj.GetChildAtIndex(0).GetValueAsUnsigned()
        self.v1 = self.valobj.GetChildAtIndex(1).GetValueAsUnsigned()
        self.v2 = self.valobj.GetChildAtIndex(2).GetValueAsUnsigned()
        self.v3 = self.valobj.GetChildAtIndex(3).GetValueAsUnsigned()

    def num_children(self):
        return 8

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        if index == 0:
            return self.valobj.CreateValueFromExpression('u8x32', 'uint16_t x[32] = {' + str(self.v0 & 0xFF) + ',' + str((self.v0 >> 8) & 0xFF) + ',' + str((self.v0 >> 16) & 0xFF) + ',' + str((self.v0 >> 24) & 0xFF) + ',' + str((self.v0 >> 32) & 0xFF) + ',' + str((self.v0 >> 40) & 0xFF) + ',' + str((self.v0 >> 48) & 0xFF) + ',' + str(self.v0 >> 56) + ',' + str(self.v1 & 0xFF) + ',' + str((self.v1 >> 8) & 0xFF) + ',' + str((self.v1 >> 16) & 0xFF) + ',' + str((self.v1 >> 24) & 0xFF) + ',' + str((self.v1 >> 32) & 0xFF) + ',' + str((self.v1 >> 40) & 0xFF) + ',' + str((self.v1 >> 48) & 0xFF) + ',' + str(self.v1 >> 56) + ',' + str(self.v2 & 0xFF) + ',' + str((self.v2 >> 8) & 0xFF) + ',' + str((self.v2 >> 16) & 0xFF) + ',' + str((self.v2 >> 24) & 0xFF) + ',' + str((self.v2 >> 32) & 0xFF) + ',' + str((self.v2 >> 40) & 0xFF) + ',' + str((self.v2 >> 48) & 0xFF) + ',' + str(self.v2 >> 56) + ',' + str(self.v3 & 0xFF) + ',' + str((self.v3 >> 8) & 0xFF) + ',' + str((self.v3 >> 16) & 0xFF) + ',' + str((self.v3 >> 24) & 0xFF) + ',' + str((self.v3 >> 32) & 0xFF) + ',' + str((self.v3 >> 40) & 0xFF) + ',' + str((self.v3 >> 48) & 0xFF) + ',' + str(self.v3 >> 56) + '}; x')
        elif index == 1:
            return self.valobj.CreateValueFromExpression('i8x32', 'int16_t x[32] = {(int8_t)' + str(self.v0 & 0xFF) + ',(int8_t)' + str((self.v0 >> 8) & 0xFF) + ',(int8_t)' + str((self.v0 >> 16) & 0xFF) + ',(int8_t)' + str((self.v0 >> 24) & 0xFF) + ',(int8_t)' + str((self.v0 >> 32) & 0xFF) + ',(int8_t)' + str((self.v0 >> 40) & 0xFF) + ',(int8_t)' + str((self.v0 >> 48) & 0xFF) + ',(int8_t)' + str(self.v0 >> 56) + ',(int8_t)' + str(self.v1 & 0xFF) + ',(int8_t)' + str((self.v1 >> 8) & 0xFF) + ',(int8_t)' + str((self.v1 >> 16) & 0xFF) + ',(int8_t)' + str((self.v1 >> 24) & 0xFF) + ',(int8_t)' + str((self.v1 >> 32) & 0xFF) + ',(int8_t)' + str((self.v1 >> 40) & 0xFF) + ',(int8_t)' + str((self.v1 >> 48) & 0xFF) + ',(int8_t)' + str(self.v1 >> 56) + ',(int8_t)' + str(self.v2 & 0xFF) + ',(int8_t)' + str((self.v2 >> 8) & 0xFF) + ',(int8_t)' + str((self.v2 >> 16) & 0xFF) + ',(int8_t)' + str((self.v2 >> 24) & 0xFF) + ',(int8_t)' + str((self.v2 >> 32) & 0xFF) + ',(int8_t)' + str((self.v2 >> 40) & 0xFF) + ',(int8_t)' + str((self.v2 >> 48) & 0xFF) + ',(int8_t)' + str(self.v2 >> 56) + ',(int8_t)' + str(self.v3 & 0xFF) + ',(int8_t)' + str((self.v3 >> 8) & 0xFF) + ',(int8_t)' + str((self.v3 >> 16) & 0xFF) + ',(int8_t)' + str((self.v3 >> 24) & 0xFF) + ',(int8_t)' + str((self.v3 >> 32) & 0xFF) + ',(int8_t)' + str((self.v3 >> 40) & 0xFF) + ',(int8_t)' + str((self.v3 >> 48) & 0xFF) + ',(int8_t)' + str(self.v3 >> 56) + '}; x')
        elif index == 2:
            return self.valobj.CreateValueFromExpression('u16x16', 'uint16_t x[16] = {' + str(self.v0 & 0xFFFF) + ',' + str((self.v0 >> 16) & 0xFFFF) + ',' + str((self.v0 >> 32) & 0xFFFF) + ',' + str(self.v0 >> 48) + ',' + str(self.v1 & 0xFFFF) + ',' + str((self.v1 >> 16) & 0xFFFF) + ',' + str((self.v1 >> 32) & 0xFFFF) + ',' + str(self.v1 >> 48) + ',' + str(self.v2 & 0xFFFF) + ',' + str((self.v2 >> 16) & 0xFFFF) + ',' + str((self.v2 >> 32) & 0xFFFF) + ',' + str(self.v2 >> 48) + ',' + str(self.v3 & 0xFFFF) + ',' + str((self.v3 >> 16) & 0xFFFF) + ',' + str((self.v3 >> 32) & 0xFFFF) + ',' + str(self.v3 >> 48) + '}; x')
        elif index == 3:
            return self.valobj.CreateValueFromExpression('i16x16', 'int16_t x[16] = {(int16_t)' + str(self.v0 & 0xFFFF) + ',(int16_t)' + str((self.v0 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v0 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v0 >> 48) + ',(int16_t)' + str(self.v1 & 0xFFFF) + ',(int16_t)' + str((self.v1 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v1 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v1 >> 48) + ',(int16_t)' + str(self.v2 & 0xFFFF) + ',(int16_t)' + str((self.v2 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v2 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v2 >> 48) + ',(int16_t)' + str(self.v3 & 0xFFFF) + ',(int16_t)' + str((self.v3 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v3 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v3 >> 48) + '}; x')
        elif index == 4:
            return self.valobj.CreateValueFromExpression('u32x8', 'uint32_t x[8] = {' + str(self.v0 & 0xFFFFFFFF) + ',' + str(self.v0 >> 32) + ',' + str(self.v1 & 0xFFFFFFFF) + ',' + str(self.v1 >> 32) + ',' + str(self.v2 & 0xFFFFFFFF) + ',' + str(self.v2 >> 32) + ',' + str(self.v3 & 0xFFFFFFFF) + ',' + str(self.v3 >> 32) + '}; x')
        elif index == 5:
            return self.valobj.CreateValueFromExpression('i32x8', 'int32_t x[8] = {(int32_t)' + str(self.v0 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v0 >> 32) + ',(int32_t)' + str(self.v1 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v1 >> 32) + ',(int32_t)' + str(self.v2 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v2 >> 32) + ',(int32_t)' + str(self.v3 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v3 >> 32) + '}; x')
        elif index == 6:
            return self.valobj.CreateValueFromExpression('u64x4', 'uint64_t x[4] = {' + str(self.v0) + ',' + str(self.v1) + ',' + str(self.v2) + ',' + str(self.v3) + '}; x')
        elif index == 7:
            return self.valobj.CreateValueFromExpression('i64x4', 'int64_t x[4] = {(int64_t)' + str(self.v0) + ',(int64_t)' + str(self.v1) + ',(int64_t)' + str(self.v2) + ',(int64_t)' + str(self.v3) + '}; x')
        else:
            return None

class Simd512Printer:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj

    def update(self):
        self.v0 = self.valobj.GetChildAtIndex(0).GetValueAsUnsigned()
        self.v1 = self.valobj.GetChildAtIndex(1).GetValueAsUnsigned()
        self.v2 = self.valobj.GetChildAtIndex(2).GetValueAsUnsigned()
        self.v3 = self.valobj.GetChildAtIndex(3).GetValueAsUnsigned()
        self.v4 = self.valobj.GetChildAtIndex(4).GetValueAsUnsigned()
        self.v5 = self.valobj.GetChildAtIndex(5).GetValueAsUnsigned()
        self.v6 = self.valobj.GetChildAtIndex(6).GetValueAsUnsigned()
        self.v7 = self.valobj.GetChildAtIndex(7).GetValueAsUnsigned()

    def num_children(self):
        return 8

    def get_child_index(self, name):
        return int(name.lstrip('[').rstrip(']'))

    def get_child_at_index(self, index):
        if index == 0:
            return self.valobj.CreateValueFromExpression('u8x64', 'uint16_t x[64] = {' + str(self.v0 & 0xFF) + ',' + str((self.v0 >> 8) & 0xFF) + ',' + str((self.v0 >> 16) & 0xFF) + ',' + str((self.v0 >> 24) & 0xFF) + ',' + str((self.v0 >> 32) & 0xFF) + ',' + str((self.v0 >> 40) & 0xFF) + ',' + str((self.v0 >> 48) & 0xFF) + ',' + str(self.v0 >> 56) + ',' + str(self.v1 & 0xFF) + ',' + str((self.v1 >> 8) & 0xFF) + ',' + str((self.v1 >> 16) & 0xFF) + ',' + str((self.v1 >> 24) & 0xFF) + ',' + str((self.v1 >> 32) & 0xFF) + ',' + str((self.v1 >> 40) & 0xFF) + ',' + str((self.v1 >> 48) & 0xFF) + ',' + str(self.v1 >> 56) + ',' + str(self.v2 & 0xFF) + ',' + str((self.v2 >> 8) & 0xFF) + ',' + str((self.v2 >> 16) & 0xFF) + ',' + str((self.v2 >> 24) & 0xFF) + ',' + str((self.v2 >> 32) & 0xFF) + ',' + str((self.v2 >> 40) & 0xFF) + ',' + str((self.v2 >> 48) & 0xFF) + ',' + str(self.v2 >> 56) + ',' + str(self.v3 & 0xFF) + ',' + str((self.v3 >> 8) & 0xFF) + ',' + str((self.v3 >> 16) & 0xFF) + ',' + str((self.v3 >> 24) & 0xFF) + ',' + str((self.v3 >> 32) & 0xFF) + ',' + str((self.v3 >> 40) & 0xFF) + ',' + str((self.v3 >> 48) & 0xFF) + ',' + str(self.v3 >> 56) + ',' + str(self.v4 & 0xFF) + ',' + str((self.v4 >> 8) & 0xFF) + ',' + str((self.v4 >> 16) & 0xFF) + ',' + str((self.v4 >> 24) & 0xFF) + ',' + str((self.v4 >> 32) & 0xFF) + ',' + str((self.v4 >> 40) & 0xFF) + ',' + str((self.v4 >> 48) & 0xFF) + ',' + str(self.v4 >> 56) + ',' + str(self.v5 & 0xFF) + ',' + str((self.v5 >> 8) & 0xFF) + ',' + str((self.v5 >> 16) & 0xFF) + ',' + str((self.v5 >> 24) & 0xFF) + ',' + str((self.v5 >> 32) & 0xFF) + ',' + str((self.v5 >> 40) & 0xFF) + ',' + str((self.v5 >> 48) & 0xFF) + ',' + str(self.v5 >> 56) + ',' + str(self.v6 & 0xFF) + ',' + str((self.v6 >> 8) & 0xFF) + ',' + str((self.v6 >> 16) & 0xFF) + ',' + str((self.v6 >> 24) & 0xFF) + ',' + str((self.v6 >> 32) & 0xFF) + ',' + str((self.v6 >> 40) & 0xFF) + ',' + str((self.v6 >> 48) & 0xFF) + ',' + str(self.v6 >> 56) + ',' + str(self.v7 & 0xFF) + ',' + str((self.v7 >> 8) & 0xFF) + ',' + str((self.v7 >> 16) & 0xFF) + ',' + str((self.v7 >> 24) & 0xFF) + ',' + str((self.v7 >> 32) & 0xFF) + ',' + str((self.v7 >> 40) & 0xFF) + ',' + str((self.v7 >> 48) & 0xFF) + ',' + str(self.v7 >> 56) + '}; x')
        elif index == 1:
            return self.valobj.CreateValueFromExpression('i8x64', 'int16_t x[64] = {(int8_t)' + str(self.v0 & 0xFF) + ',(int8_t)' + str((self.v0 >> 8) & 0xFF) + ',(int8_t)' + str((self.v0 >> 16) & 0xFF) + ',(int8_t)' + str((self.v0 >> 24) & 0xFF) + ',(int8_t)' + str((self.v0 >> 32) & 0xFF) + ',(int8_t)' + str((self.v0 >> 40) & 0xFF) + ',(int8_t)' + str((self.v0 >> 48) & 0xFF) + ',(int8_t)' + str(self.v0 >> 56) + ',(int8_t)' + str(self.v1 & 0xFF) + ',(int8_t)' + str((self.v1 >> 8) & 0xFF) + ',(int8_t)' + str((self.v1 >> 16) & 0xFF) + ',(int8_t)' + str((self.v1 >> 24) & 0xFF) + ',(int8_t)' + str((self.v1 >> 32) & 0xFF) + ',(int8_t)' + str((self.v1 >> 40) & 0xFF) + ',(int8_t)' + str((self.v1 >> 48) & 0xFF) + ',(int8_t)' + str(self.v1 >> 56) + ',(int8_t)' + str(self.v2 & 0xFF) + ',(int8_t)' + str((self.v2 >> 8) & 0xFF) + ',(int8_t)' + str((self.v2 >> 16) & 0xFF) + ',(int8_t)' + str((self.v2 >> 24) & 0xFF) + ',(int8_t)' + str((self.v2 >> 32) & 0xFF) + ',(int8_t)' + str((self.v2 >> 40) & 0xFF) + ',(int8_t)' + str((self.v2 >> 48) & 0xFF) + ',(int8_t)' + str(self.v2 >> 56) + ',(int8_t)' + str(self.v3 & 0xFF) + ',(int8_t)' + str((self.v3 >> 8) & 0xFF) + ',(int8_t)' + str((self.v3 >> 16) & 0xFF) + ',(int8_t)' + str((self.v3 >> 24) & 0xFF) + ',(int8_t)' + str((self.v3 >> 32) & 0xFF) + ',(int8_t)' + str((self.v3 >> 40) & 0xFF) + ',(int8_t)' + str((self.v3 >> 48) & 0xFF) + ',(int8_t)' + str(self.v3 >> 56) + ',(int8_t)' + str(self.v4 & 0xFF) + ',(int8_t)' + str((self.v4 >> 8) & 0xFF) + ',(int8_t)' + str((self.v4 >> 16) & 0xFF) + ',(int8_t)' + str((self.v4 >> 24) & 0xFF) + ',(int8_t)' + str((self.v4 >> 32) & 0xFF) + ',(int8_t)' + str((self.v4 >> 40) & 0xFF) + ',(int8_t)' + str((self.v4 >> 48) & 0xFF) + ',(int8_t)' + str(self.v4 >> 56) + ',(int8_t)' + str(self.v5 & 0xFF) + ',(int8_t)' + str((self.v5 >> 8) & 0xFF) + ',(int8_t)' + str((self.v5 >> 16) & 0xFF) + ',(int8_t)' + str((self.v5 >> 24) & 0xFF) + ',(int8_t)' + str((self.v5 >> 32) & 0xFF) + ',(int8_t)' + str((self.v5 >> 40) & 0xFF) + ',(int8_t)' + str((self.v5 >> 48) & 0xFF) + ',(int8_t)' + str(self.v5 >> 56) + ',(int8_t)' + str(self.v6 & 0xFF) + ',(int8_t)' + str((self.v6 >> 8) & 0xFF) + ',(int8_t)' + str((self.v6 >> 16) & 0xFF) + ',(int8_t)' + str((self.v6 >> 24) & 0xFF) + ',(int8_t)' + str((self.v6 >> 32) & 0xFF) + ',(int8_t)' + str((self.v6 >> 40) & 0xFF) + ',(int8_t)' + str((self.v6 >> 48) & 0xFF) + ',(int8_t)' + str(self.v6 >> 56) + ',(int8_t)' + str(self.v7 & 0xFF) + ',(int8_t)' + str((self.v7 >> 8) & 0xFF) + ',(int8_t)' + str((self.v7 >> 16) & 0xFF) + ',(int8_t)' + str((self.v7 >> 24) & 0xFF) + ',(int8_t)' + str((self.v7 >> 32) & 0xFF) + ',(int8_t)' + str((self.v7 >> 40) & 0xFF) + ',(int8_t)' + str((self.v7 >> 48) & 0xFF) + ',(int8_t)' + str(self.v7 >> 56) + '}; x')
        elif index == 2:
            return self.valobj.CreateValueFromExpression('u16x32', 'uint16_t x[32] = {' + str(self.v0 & 0xFFFF) + ',' + str((self.v0 >> 16) & 0xFFFF) + ',' + str((self.v0 >> 32) & 0xFFFF) + ',' + str(self.v0 >> 48) + ',' + str(self.v1 & 0xFFFF) + ',' + str((self.v1 >> 16) & 0xFFFF) + ',' + str((self.v1 >> 32) & 0xFFFF) + ',' + str(self.v1 >> 48) + ',' + str(self.v2 & 0xFFFF) + ',' + str((self.v2 >> 16) & 0xFFFF) + ',' + str((self.v2 >> 32) & 0xFFFF) + ',' + str(self.v2 >> 48) + ',' + str(self.v3 & 0xFFFF) + ',' + str((self.v3 >> 16) & 0xFFFF) + ',' + str((self.v3 >> 32) & 0xFFFF) + ',' + str(self.v3 >> 48) + ',' + str(self.v4 & 0xFFFF) + ',' + str((self.v4 >> 16) & 0xFFFF) + ',' + str((self.v4 >> 32) & 0xFFFF) + ',' + str(self.v4 >> 48) + ',' + str(self.v5 & 0xFFFF) + ',' + str((self.v5 >> 16) & 0xFFFF) + ',' + str((self.v5 >> 32) & 0xFFFF) + ',' + str(self.v5 >> 48) + ',' + str(self.v6 & 0xFFFF) + ',' + str((self.v6 >> 16) & 0xFFFF) + ',' + str((self.v6 >> 32) & 0xFFFF) + ',' + str(self.v6 >> 48) + ',' + str(self.v7 & 0xFFFF) + ',' + str((self.v7 >> 16) & 0xFFFF) + ',' + str((self.v7 >> 32) & 0xFFFF) + ',' + str(self.v7 >> 48) + '}; x')
        elif index == 3:
            return self.valobj.CreateValueFromExpression('i16x32', 'int16_t x[32] = {(int16_t)' + str(self.v0 & 0xFFFF) + ',(int16_t)' + str((self.v0 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v0 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v0 >> 48) + ',(int16_t)' + str(self.v1 & 0xFFFF) + ',(int16_t)' + str((self.v1 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v1 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v1 >> 48) + ',(int16_t)' + str(self.v2 & 0xFFFF) + ',(int16_t)' + str((self.v2 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v2 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v2 >> 48) + ',(int16_t)' + str(self.v3 & 0xFFFF) + ',(int16_t)' + str((self.v3 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v3 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v3 >> 48) + ',(int16_t)' + str(self.v4 & 0xFFFF) + ',(int16_t)' + str((self.v4 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v4 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v4 >> 48) + ',(int16_t)' + str(self.v5 & 0xFFFF) + ',(int16_t)' + str((self.v5 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v5 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v5 >> 48) + ',(int16_t)' + str(self.v6 & 0xFFFF) + ',(int16_t)' + str((self.v6 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v6 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v6 >> 48) + ',(int16_t)' + str(self.v7 & 0xFFFF) + ',(int16_t)' + str((self.v7 >> 16) & 0xFFFF) + ',(int16_t)' + str((self.v7 >> 32) & 0xFFFF) + ',(int16_t)' + str(self.v7 >> 48) + '}; x')
        elif index == 4:
            return self.valobj.CreateValueFromExpression('u32x16', 'uint32_t x[16] = {' + str(self.v0 & 0xFFFFFFFF) + ',' + str(self.v0 >> 32) + ',' + str(self.v1 & 0xFFFFFFFF) + ',' + str(self.v1 >> 32) + ',' + str(self.v2 & 0xFFFFFFFF) + ',' + str(self.v2 >> 32) + ',' + str(self.v3 & 0xFFFFFFFF) + ',' + str(self.v3 >> 32) + ',' + str(self.v4 & 0xFFFFFFFF) + ',' + str(self.v4 >> 32) + ',' + str(self.v5 & 0xFFFFFFFF) + ',' + str(self.v5 >> 32) + ',' + str(self.v6 & 0xFFFFFFFF) + ',' + str(self.v6 >> 32) + ',' + str(self.v7 & 0xFFFFFFFF) + ',' + str(self.v7 >> 32) + '}; x')
        elif index == 5:
            return self.valobj.CreateValueFromExpression('i32x16', 'int32_t x[16] = {(int32_t)' + str(self.v0 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v0 >> 32) + ',(int32_t)' + str(self.v1 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v1 >> 32) + ',(int32_t)' + str(self.v2 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v2 >> 32) + ',(int32_t)' + str(self.v3 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v3 >> 32) + ',(int32_t)' + str(self.v4 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v4 >> 32) + ',(int32_t)' + str(self.v5 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v5 >> 32) + ',(int32_t)' + str(self.v6 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v6 >> 32) + ',(int32_t)' + str(self.v7 & 0xFFFFFFFF) + ',(int32_t)' + str(self.v7 >> 32) + '}; x')
        elif index == 6:
            return self.valobj.CreateValueFromExpression('u64x8', 'uint64_t x[8] = {' + str(self.v0) + ',' + str(self.v1) + ',' + str(self.v2) + ',' + str(self.v3) + ',' + str(self.v4) + ',' + str(self.v5) + ',' + str(self.v6) + ',' + str(self.v7) + '}; x')
        elif index == 7:
            return self.valobj.CreateValueFromExpression('i64x8', 'int64_t x[8] = {(int64_t)' + str(self.v0) + ',(int64_t)' + str(self.v1) + ',(int64_t)' + str(self.v2) + ',(int64_t)' + str(self.v3) + ',(int64_t)' + str(self.v4) + ',(int64_t)' + str(self.v5) + ',(int64_t)' + str(self.v6) + ',(int64_t)' + str(self.v7) + '}; x')
        else:
            return None

def __lldb_init_module(debugger, dict):
    debugger.HandleCommand('type synthetic add -w simd -l simd.Simd128Printer __m128i')
    debugger.HandleCommand('type synthetic add -w simd -l simd.Simd256Printer __m256i')
    debugger.HandleCommand('type synthetic add -w simd -l simd.Simd512Printer __m512i')
    debugger.HandleCommand('type category enable simd')
    debugger.HandleCommand('type category disable VectorTypes')
