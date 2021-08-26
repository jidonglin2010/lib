#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <tuple>
#include <assert.h>

#include "cFpData.h"


namespace nFp {



void cFpData::Print() {
    printf("sign:%0d exp:%0d mant:0x%llx mant_ext:0x%llx sticky:%0d, double:%f\n", sign, exp, mant, mant_ext, sticky, to_double());


}


cFpData::cFpData() {
    sign = 0;
    exp = 0;
    mant = 0;
    mant_ext = 0;
    sticky = 0;
    round_mode = ROUND_NEAREST_EVEN;
}
cFpData::cFpData(double number) : cFpData() {
    uint64_t numberInt = *(uint64_t*)&number;
    sign = numberInt >> 63;
    exp = (numberInt & DP_EXP_MASK) >> DP_EXP_LSB;
    if (exp == 0) {
        mant = (numberInt & DP_MANT_MASK) << 12;
    }
    else {
        mant = (numberInt & DP_MANT_MASK) << 11;
        mant = mant | UINT64_MSB_ONE;
    }
    if (mant != 0) {
        exp = exp - DP_EXP_BIAS;
    }

    mant_ext = 0;
    sticky = 0;
};

cFpData::cFpData(float number) : cFpData() {
    uint32_t numberInt32 = *(uint32_t*)&number;
    uint64_t numberInt = numberInt32;
    sign = numberInt >> 31;
    exp = (numberInt & SP_EXP_MASK) >> SP_EXP_LSB;
    if (exp == 0) {
        mant = (numberInt & SP_MANT_MASK) << 9;
    }
    else {
        mant = (numberInt & SP_MANT_MASK) << 8;
        mant = mant | UINT64_MSB_ONE;
    }

    if (mant != 0) {
        exp = exp - SP_EXP_BIAS;
    }
    mant_ext = 0;
    sticky = 0;
};

cFpData::cFpData(uint64_t number) : cFpData() {
    if (number == 0) return;

    sign = 0;
    exp = 63;
    mant = number;
    mant_ext = 0;
    sticky = 0;

    Normalize();
}

cFpData::cFpData(uint32_t number) : cFpData((uint64_t)number) {
    return;
}

cFpData::cFpData(int64_t number) : cFpData() {

    if (number < 0) {
        sign = 1;
    }
    else {
        sign = 0;
    }

    if (number == 0) return;

    exp = 63;

    if (number == 0x8000000000000000LL || number >= 0) {
        mant = (uint64_t)number;
    }
    else {
        mant = (uint64_t)-number;
    }

    mant_ext = 0;
    sticky = 0;

    Normalize();
}



cFpData::cFpData(int32_t number) : cFpData((int64_t)number) {
    return;
}

cFpData::cFpData(bool sign, int32_t exp, uint64_t mant, uint64_t mant_ext, bool sticky) : cFpData() {
    this->sign = sign;
    this->exp = exp;
    this->mant = mant;
    this->mant_ext = mant_ext;
    this->sticky = sticky;
}

double cFpData::to_double() const {
    if (!isNumerical()) {
        if (isInf()) {
            if (sign) {
                uint64_t nInf = 0xfff0000000000000ULL;
                return *(double*)&nInf;
            }
            else {
                uint64_t pInf = 0x7ff0000000000000ULL;
                return *(double*)&pInf;
            }
        }
        return 0;
    }
    uint64_t numberInt = 0;

    cFpData tmpResult = *this;
    tmpResult.Normalize();
    tmpResult.RoundDP();

    if (tmpResult.abs() > GetFpDataConst(DP_POS_MAX_NORMAL_CONST)) {
        return 0;
    }
    else if (tmpResult.abs() >= GetFpDataConst(DP_POS_MIN_NORMAL_CONST)) {
        numberInt = tmpResult.sign ? UINT64_MSB_ONE : 0;
        numberInt = numberInt + (((uint64_t)tmpResult.exp + DP_EXP_BIAS) << DP_EXP_LSB);
        numberInt = numberInt + ((tmpResult.mant >> 11) & DP_MANT_MASK);
    }
    else if (tmpResult.abs() >= GetFpDataConst(DP_POS_MIN_DENORMAL_CONST)) {
        numberInt = tmpResult.sign ? UINT64_MSB_ONE : 0;
        numberInt = numberInt + (((tmpResult.mant) >> 12) & DP_MANT_MASK);
    }
    else {
        numberInt = tmpResult.sign ? UINT64_MSB_ONE : 0;
    }

    double result = *(double*)&numberInt;

    //printf("numberInt:0x%llx\n", numberInt);

    return result;
}

cFpData& cFpData::operator=(const cFpData& fpData) {
    sign = fpData.sign;
    exp = fpData.exp;
    mant = fpData.mant;
    mant_ext = fpData.mant_ext;
    sticky = fpData.sticky;
    round_mode = fpData.round_mode;

    return *this;
}

bool cFpData::operator>(const cFpData& fpData) const {
    return isGreater(fpData);
}

bool cFpData::operator<(const cFpData& fpData) const {
    return isSmaller(fpData);
}

bool cFpData::operator==(const cFpData& fpData) const {
    //printf("operator==\n");
    return isEqual(fpData);
}

bool cFpData::operator!=(const cFpData& fpData) const {
    //printf("operator!=\n");
    return !isEqual(fpData);
}

bool cFpData::operator>=(const cFpData& fpData) const {
    return isGreater(fpData) || isEqual(fpData);
}

bool cFpData::operator<=(const cFpData& fpData) const {
    return isSmaller(fpData) || isEqual(fpData);
}

cFpData cFpData::operator+(const cFpData& fpData) const {
    return add(fpData);
}

cFpData cFpData::operator-(const cFpData& fpData) const {
    return sub(fpData);
}

cFpData cFpData::operator*(const cFpData& fpData) const {
    return mul(fpData);
}

cFpData cFpData::operator/(const cFpData& fpData) const {
    return div(fpData);
}

// Category
bool cFpData::isPos() const {
    return sign == 0;
};
bool cFpData::isNeg() const {
    return sign == 1;
};

bool cFpData::isNan() const {
    if (exp == EXP_MAX && (mant != 0 || mant_ext != 0)) {
        return true;
    }
    else {
        return false;
    }
};

bool cFpData::isInf() const {
    if (exp == EXP_MAX && mant == 0 && mant_ext == 0) {
        return true;
    }
    else {
        return false;
    }
};
bool cFpData::isZero() const {
    if (exp == 0 && mant == 0 && mant_ext == 0 && sticky == 0) {
        return true;
    }
    else {
        return false;
    }
};
bool cFpData::isDenormal() const {
    if (BIAS == 0) {
        return false;
    }
    else {
        return (exp == 0) && !isZero();

    }
};

bool cFpData::isNumerical() const {
    return (!isNan() && !isInf());
};
bool cFpData::isMantMsbOne() const {
    return (mant & UINT64_MSB_ONE) != 0;
};
bool cFpData::isNormalized() const {
    return isNumerical() && isMantMsbOne();
};

// Operation
cFpData cFpData::addCore(const cFpData& operand0, const cFpData& operand1) const {
    cFpData tmpOperand0 = operand0;
    cFpData tmpOperand1 = operand1;

    if (tmpOperand0.isZero() && tmpOperand1.isZero()) {
        return tmpOperand0;
    }
    else if (tmpOperand0.isZero()) {
        return tmpOperand1;
    }
    else if (tmpOperand1.isZero()) {
        return tmpOperand0;
    }

    tmpOperand0.Normalize();
    tmpOperand1.Normalize();

    if (tmpOperand0.exp > tmpOperand1.exp) {
        while (tmpOperand0.exp > tmpOperand1.exp) {
            tmpOperand1.IncrExpRightShiftMant();
        }
    }
    else if (tmpOperand0.exp < tmpOperand1.exp) {
        while (tmpOperand1.exp > tmpOperand0.exp) {
            tmpOperand0.IncrExpRightShiftMant();
        }
    }

    cFpData tmpResult;
    if (tmpOperand0.sign == tmpOperand1.sign) {
        tmpResult.sign = tmpOperand0.sign;

        bool sticky_result = tmpOperand0.sticky ^ tmpOperand1.sticky;
        bool sticky_carry = tmpOperand0.sticky & tmpOperand1.sticky;

        bool mant_ext_carry = false;
        uint64_t mant_ext_sum = 0;
        bool mant_carry = false;
        uint64_t mant_sum = 0;
        std::tie(mant_ext_carry, mant_ext_sum) = add64_carry(tmpOperand0.mant_ext, tmpOperand1.mant_ext, sticky_carry);
        std::tie(mant_carry, mant_sum) = add64_carry(tmpOperand0.mant, tmpOperand1.mant, mant_ext_carry);

        tmpResult.exp = tmpOperand0.exp;
        tmpResult.mant = mant_sum;
        tmpResult.mant_ext = mant_ext_sum;
        tmpResult.sticky = sticky_result;

        if (mant_carry) {
            tmpResult.IncrExpRightShiftMant();
            tmpResult.mant = tmpResult.mant | UINT64_MSB_ONE;
        }

        tmpResult.Normalize();
    }
    else if ((tmpOperand0.sign == 0 && tmpOperand1.sign == 1) ||
        (tmpOperand0.sign == 1 && tmpOperand1.sign == 0)) {

        if (tmpOperand0.mant == tmpOperand1.mant &&
            tmpOperand0.mant_ext == tmpOperand1.mant_ext &&
            tmpOperand0.sticky == tmpOperand1.sticky) {
            tmpResult.sign = tmpOperand0.sign;
            tmpResult.exp = 0;
            tmpResult.mant = 0;
            tmpResult.mant_ext = 0;
            tmpResult.sticky = 0;

            return tmpResult;
        }

        cFpData tmpOperandPos;
        cFpData tmpOperandNeg;
        if (tmpOperand0.sign == 0) {
            tmpOperandPos = tmpOperand0;
            tmpOperandNeg = tmpOperand1;
        }
        else {
            tmpOperandPos = tmpOperand1;
            tmpOperandNeg = tmpOperand0;
        }

        tmpOperandNeg.mant = ~tmpOperandNeg.mant;
        tmpOperandNeg.mant_ext = ~tmpOperandNeg.mant_ext;
        tmpOperandNeg.sticky = !tmpOperandNeg.sticky;

        bool sticky_result = tmpOperandPos.sticky ^ tmpOperandNeg.sticky ^ 1;
        bool sticky_carry = tmpOperandPos.sticky | tmpOperandNeg.sticky;

        bool mant_ext_carry = false;
        uint64_t mant_ext_sum = 0;
        bool mant_carry = false;
        uint64_t mant_sum = 0;
        std::tie(mant_ext_carry, mant_ext_sum) = add64_carry(tmpOperandPos.mant_ext, tmpOperandNeg.mant_ext, sticky_carry);
        std::tie(mant_carry, mant_sum) = add64_carry(tmpOperandPos.mant, tmpOperandNeg.mant, mant_ext_carry);

        tmpResult.exp = tmpOperandPos.exp;

        if (mant_carry) {
            tmpResult.sign = tmpOperandPos.sign;
            tmpResult.mant = mant_sum;
            tmpResult.mant_ext = mant_ext_sum;
            tmpResult.sticky = sticky_result;

        }
        else {
            tmpResult.sign = tmpOperandNeg.sign;
            bool discard_carry;
            std::tie(mant_ext_carry, tmpResult.mant_ext) = add64_carry(~mant_ext_sum, !sticky_carry, 1);
            std::tie(discard_carry, tmpResult.mant) = add64_carry(0, ~mant_sum, mant_ext_carry);
        }
        tmpResult.Normalize();
    }
    else {
        assert(0);
    }

    return tmpResult;
}

cFpData cFpData::mulCore(const cFpData& operand0, const cFpData& operand1) const {
    cFpData tmpOperand0 = operand0;
    cFpData tmpOperand1 = operand1;

    if (tmpOperand0.isZero()) {
        tmpOperand0.sign = tmpOperand0.sign ^ tmpOperand1.sign;
        return tmpOperand0;
    }
    if (tmpOperand1.isZero()) {
        tmpOperand1.sign = tmpOperand0.sign ^ tmpOperand1.sign;
        return tmpOperand1;
    }

    tmpOperand0.Normalize();
    tmpOperand1.Normalize();

    assert(operand0.mant_ext == 0);
    assert(operand0.sticky == 0);
    assert(operand1.mant_ext == 0);
    assert(operand1.sticky == 0);

    cFpData tmpResult;
    tmpResult.sign = tmpOperand0.sign ^ tmpOperand1.sign;
    tmpResult.mant = mul64_upper(tmpOperand0.mant, tmpOperand1.mant);
    //printf("mul64_upper:0x%llx\n", tmpResult.mant);
    tmpResult.mant_ext = mul64_lower(tmpOperand0.mant, tmpOperand1.mant);
    tmpResult.exp = tmpOperand0.exp + tmpOperand1.exp + 1;

    tmpResult.Normalize();

    return tmpResult;

}

cFpData cFpData::add(const cFpData& fpData) const {
    if (isNumerical() && fpData.isNumerical()) {
        return addCore(*this, fpData);
    }

    cFpData tmpResult;


    return tmpResult;
};
cFpData cFpData::sub(const cFpData& fpData) const {
    if (isNumerical() && fpData.isNumerical()) {
        return addCore(*this, fpData.neg());
    }

    cFpData tmpResult;

    return tmpResult;
};
cFpData cFpData::mul(const cFpData& fpData) const {
    if (isNumerical() && fpData.isNumerical()) {
        return mulCore(*this, fpData);
    }

    cFpData tmpResult;

    return tmpResult;

};
cFpData cFpData::div(const cFpData& fpData) const {
    cFpData tmpResult;

    if (!isNumerical() || !fpData.isNumerical()) {
        return tmpResult;
    }

    if (!isZero() && fpData.isZero()) {
        tmpResult.SetInf(sign ^ fpData.sign);
        return tmpResult;
    }

    if (isZero() && !fpData.isZero()) {
        tmpResult.SetZero(sign ^ fpData.sign);
        return tmpResult;
    }

    if (isZero() && fpData.isZero()) {
        tmpResult.SetSNan();
        return tmpResult;
    }

    assert(mant_ext == 0 && fpData.mant_ext == 0 && sticky == 0 && fpData.sticky == 0);

    cFpData tmpOperand0 = *this;
    cFpData tmpOperand1 = fpData;

    tmpOperand0.Normalize();
    tmpOperand1.Normalize();

    const uint32_t numBits = 64;

    uint64_t remainder = tmpOperand0.mant;
    uint64_t quotient = 0;


    bool carry = false;

    for (uint32_t i = 0; i < numBits; i++) {
        if (carry) {

            quotient = quotient | (UINT64_MSB_ONE >> i);

            remainder = remainder - (tmpOperand1.mant - remainder);

            if ((remainder & UINT64_MSB_ONE) != 0) {
                carry = 1;
            }
            else {
                remainder = remainder << 1;
                carry = 0;
            }
        }
        else if (remainder > tmpOperand1.mant) {
            quotient = quotient | (UINT64_MSB_ONE >> i);
            remainder = (remainder - tmpOperand1.mant) << 1;
        }
        else if (remainder < tmpOperand1.mant) {
            if ((remainder & UINT64_MSB_ONE) != 0) {
                carry = 1;
            }
            else {
                remainder = remainder << 1;
            }
        }
        else if (remainder == tmpOperand1.mant) {
            quotient = quotient | (UINT64_MSB_ONE >> i);
            remainder = 0;
            break;
        }
    }

    tmpResult.sign = tmpOperand0.sign ^ tmpOperand1.sign;
    tmpResult.exp = tmpOperand0.exp - tmpOperand1.exp;
    tmpResult.mant = quotient;
    tmpResult.mant_ext = 0;
    tmpResult.sticky = remainder != 0 ? 1 : 0;


    assert(tmpResult.mant != 0 || tmpResult.mant_ext != 0 || sticky);

    tmpResult.Normalize();


    return tmpResult;
};
cFpData cFpData::abs() const {
    cFpData tmpResult = *this;
    tmpResult.sign = 0;
    return tmpResult;
};




cFpData cFpData::neg() const {
    cFpData tmpResult = *this;
    tmpResult.sign = !tmpResult.sign;
    return tmpResult;
};


// Compare
bool cFpData::isGreater(const cFpData& fpData) const {
    if (!isNumerical()) return false;
    if (!fpData.isNumerical()) return false;

    // Zero handling - need Zero handling because exp == 0 is a special case
    if (isZero() && fpData.isZero()) return false;
    if (isZero() && fpData.sign) return true;
    if (isZero() && !fpData.sign) return false;
    if (sign && fpData.isZero()) return false;
    if (!sign && fpData.isZero()) return true;

    cFpData thisNumber = *this;
    cFpData cmpNumber = fpData;

    if (sign == 0 && fpData.sign == 1) {
        return true;
    }
    else if (sign == 1 && fpData.sign == 0) {
        return false;
    }
    else if (sign == 0 && fpData.sign == 0) {
        if (thisNumber.exp > cmpNumber.exp) {
            return true;
        }
        else if (thisNumber.exp == cmpNumber.exp) {
            if (thisNumber.mant > cmpNumber.mant) {
                return true;
            }
            else if (thisNumber.mant == cmpNumber.mant) {
                if (thisNumber.mant_ext > cmpNumber.mant_ext) {
                    return true;
                }
                else if (thisNumber.mant_ext == cmpNumber.mant_ext) {
                    if (thisNumber.sticky > cmpNumber.sticky) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    else if (sign == 1 && fpData.sign == 1) {
        if (thisNumber.exp < cmpNumber.exp) {
            return true;
        }
        else if (thisNumber.exp == cmpNumber.exp) {
            if (thisNumber.mant < cmpNumber.mant) {
                return true;
            }
            else if (thisNumber.mant == cmpNumber.mant) {
                if (thisNumber.mant_ext < cmpNumber.mant_ext) {
                    return true;
                }
                else if (thisNumber.mant_ext == cmpNumber.mant_ext) {
                    if (thisNumber.sticky < cmpNumber.sticky) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    else {

        assert(0);
    }


};
bool cFpData::isSmaller(const cFpData& fpData) const {
    if (!isNumerical()) return false;
    if (!fpData.isNumerical()) return false;

    // Zero handling - need Zero handling because exp == 0 is a special case
    if (isZero() && fpData.isZero()) return false;
    if (isZero() && fpData.sign) return false;
    if (isZero() && !fpData.sign) return true;
    if (sign && fpData.isZero()) return true;
    if (!sign && fpData.isZero()) return false;

    cFpData thisNumber = *this;
    cFpData cmpNumber = fpData;

    if (sign == 1 && fpData.sign == 0) {
        return true;
    }
    else if (sign == 0 && fpData.sign == 1) {
        return false;
    }
    else if (sign == 0 && fpData.sign == 0) {
        if (thisNumber.exp < cmpNumber.exp) {
            return true;
        }
        else if (thisNumber.exp == cmpNumber.exp) {
            if (thisNumber.mant < cmpNumber.mant) {
                return true;
            }
            else if (thisNumber.mant == cmpNumber.mant) {
                if (thisNumber.mant_ext < cmpNumber.mant_ext) {
                    return true;
                }
                else if (thisNumber.mant_ext == cmpNumber.mant_ext) {
                    if (thisNumber.sticky < cmpNumber.sticky) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    else if (sign == 1 && fpData.sign == 1) {
        if (thisNumber.exp > cmpNumber.exp) {
            return true;
        }
        else if (thisNumber.exp == cmpNumber.exp) {
            if (thisNumber.mant > cmpNumber.mant) {
                return true;
            }
            else if (thisNumber.mant == cmpNumber.mant) {
                if (thisNumber.mant_ext > cmpNumber.mant_ext) {
                    return true;
                }
                else if (thisNumber.mant_ext == cmpNumber.mant_ext) {
                    if (thisNumber.sticky > cmpNumber.sticky) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    else {
        assert(0);
    }
};
bool cFpData::isEqual(const cFpData& fpData) const {
    if (!isNumerical()) return false;
    if (!fpData.isNumerical()) return false;



    // +0 and -0 are equal
    if (isZero() && fpData.isZero()) return true;

    cFpData thisNumber = *this;
    cFpData cmpNumber = fpData;

    thisNumber.Normalize();
    cmpNumber.Normalize();

    if (thisNumber.sign == cmpNumber.sign &&
        thisNumber.exp == cmpNumber.exp &&
        thisNumber.mant == cmpNumber.mant &&
        thisNumber.mant_ext == cmpNumber.mant_ext &&
        thisNumber.sticky == cmpNumber.sticky) {
        return true;
    }
    else {
        return false;
    }
    assert(0);

    return false;

};
bool cFpData::isGreaterEqual(const cFpData& fpData) const {
    return isGreater(fpData) || isEqual(fpData);
}
bool cFpData::isSmallerEqual(const cFpData& fpData) const {
    return isSmaller(fpData) || isEqual(fpData);
};



// Manipulate
bool cFpData::getMantLsb() const {
    return mant & 0x1;
}
bool cFpData::getMantExtLsb() const {
    return mant_ext & 0x1;
}
bool cFpData::isMantExtMsbOne() const {
    return (mant_ext & UINT64_MSB_ONE) != 0;
}
void cFpData::Normalize() {
    if (!isNumerical()) return;
    if (isZero()) return;

    assert(mant != 0 || mant_ext != 0 || sticky);

    while (!isNormalized()) {
        DecExpLeftShiftMant();
    }
};
void cFpData::IncrExpRightShiftMant() {
    if (!isNumerical()) return;
    if (isZero()) return;

    exp++;
    bool mantLsb = getMantLsb();
    bool mantExtLsb = getMantExtLsb();
    sticky = sticky | mantExtLsb;
    mant = mant >> 1;
    mant_ext = mant_ext >> 1;
    mant_ext = mantLsb ? (mant_ext | UINT64_MSB_ONE) : mant_ext;
};
void cFpData::DecExpLeftShiftMant() {
    if (!isNumerical()) return;
    if (isZero()) return;
    if (isMantMsbOne()) return;

    exp--;
    mant_ext = mant_ext << 1;
    if (sticky) {
        mant_ext = mant_ext | 0x1;
        sticky = 0;
    }
    mant = mant << 1;
    mant = isMantExtMsbOne() ? (mant | 0x1ULL) : mant;
};

void cFpData::SetInf(bool sign) {
    this->sign = sign;
    this->exp = EXP_MAX;
    this->mant = 0;
    this->mant_ext = 0;
    this->sticky = 0;
}



void cFpData::SetZero(bool sign) {
    this->sign = sign;
    this->exp = 0;
    this->mant = 0;
    this->mant_ext = 0;
    this->sticky = 0;
}


void cFpData::SetSNan(bool sign) {
    this->sign = sign;
    this->exp = EXP_MAX;
    this->mant = UINT64_MSB_ONE | 0x1;
    this->mant_ext = 0;
    this->sticky = 0;
}



void cFpData::SetQNan(bool sign) {
    this->sign = sign;
    this->exp = EXP_MAX;
    this->mant = 0x1;
    this->mant_ext = 0;
    this->sticky = 0;
}

void cFpData::Round(uint32_t guard_position) { // Count from MSB of mant
    assert(guard_position > 0 && guard_position < 64);

    bool guard_bit = 0;
    bool round_bit = 0;
    bool sticky_bit = 0;

    if (guard_position == 63) {
        round_bit = (mant_ext >> 63) & 0x1;
        sticky_bit = (mant_ext & 0x7FFFFFFFFFFFFFFFULL) != 0;
    }
    else if (guard_position == 62) {
        round_bit = mant & 0x1;
        sticky_bit = mant_ext != 0;
    }
    else {
        uint32_t round_shift = 63 - (guard_position + 1);
        uint64_t sticky_mask = 0xFFFFFFFFFFFFFFFFULL >> (guard_position + 2);
        round_bit = (mant >> round_shift) & 0x1;
        sticky_bit = ((mant & sticky_mask) | mant_ext) != 0;
    }

    sticky_bit = sticky_bit | this->sticky;

    uint32_t guard_shift = 63 - guard_position;
    guard_bit = (mant >> guard_shift) & 0x1;

    bool guard_carry_bit = roundCore(this->sign, guard_bit, round_bit, sticky_bit, round_mode);

    uint64_t guard_carry = guard_carry_bit ? 0x1ULL << (63 - guard_position) : 0;

    //printf("mant before: 0x%llx\n", mant);
    //printf("guard_bit:%0d, round_bit:%0d, sticky_bit:%0d, guard_carry_bit:%0d\n", guard_bit, round_bit, sticky_bit, guard_carry_bit);
    mant = mant & (0xFFFFFFFFFFFFFFFFULL >> guard_shift << guard_shift); // Discard bits after guard bit
    //printf("mant after : 0x%llx\n", mant);
    mant_ext = 0;
    sticky = 0;

    bool carry = 0;

    std::tie(carry, mant) = add64_carry(mant, guard_carry, 0);

    if (carry) {
        IncrExpRightShiftMant();
        mant = mant & UINT64_MSB_ONE;
    }
    //printf("mant after carry: 0x%llx\n", mant);

}

bool cFpData::roundCore(bool sign, bool guard, bool round, bool sticky, eRoundMode rmode) {
    bool carry = 0;

                                                //    V - guard
    if (!sign && !guard && !round && !sticky) { // +1.000< - sticky
        carry = 0;                              //     ^ - round
    }
    else if (!sign && !guard && !round && sticky) { // +1.001
        if (rmode == ROUND_POS_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (!sign && !guard && round && !sticky) { // +1.010
        if (rmode == ROUND_NEAREST_ODD ||
            rmode == ROUND_POS_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (!sign && !guard && round && sticky) { // +1.011
        if (rmode == ROUND_NEAREST_EVEN ||
            rmode == ROUND_NEAREST_ODD ||
            rmode == ROUND_POS_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (!sign && guard && !round && !sticky) { // +1.100
        carry = 0;
    }
    else if (!sign && guard && !round && sticky) { // +1.101
        if (rmode == ROUND_POS_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (!sign && guard && round && !sticky) { // +1.110
        if (rmode == ROUND_NEAREST_EVEN ||
            rmode == ROUND_POS_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (!sign && guard && round && sticky) { // +1.111
        if (rmode == ROUND_NEAREST_EVEN ||
            rmode == ROUND_NEAREST_ODD ||
            rmode == ROUND_POS_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (sign && !guard && !round && !sticky) { // -1.000
        carry = 0;
    }
    else if (sign && !guard && !round && sticky) { // -1.001
        if (rmode == ROUND_NEG_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (sign && !guard && round && !sticky) { // -1.010
        if (rmode == ROUND_NEAREST_ODD ||
            rmode == ROUND_NEG_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (sign && !guard && round && sticky) { // -1.011
        if (rmode == ROUND_NEAREST_EVEN ||
            rmode == ROUND_NEAREST_ODD ||
            rmode == ROUND_NEG_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (sign && guard && !round && !sticky) { // -1.100
        carry = 0;
    }
    else if (sign && guard && !round && sticky) { // -1.101
        if (rmode == ROUND_NEG_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (sign && guard && round && !sticky) { // -1.110
        if (rmode == ROUND_NEAREST_EVEN ||
            rmode == ROUND_NEG_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }
    else if (sign && guard && round && sticky) { // -1.111
        if (rmode == ROUND_NEAREST_EVEN ||
            rmode == ROUND_NEAREST_ODD ||
            rmode == ROUND_NEG_INF ||
            rmode == ROUND_AWAY_ZERO) {
            carry = 1;
        }
    }

    return carry;
}

void cFpData::RoundDP() {
    Round(DP_GUARD_POSITION);
}

void cFpData::RoundSP() {
    Round(SP_GUARD_POSITION);
}

std::tuple<bool, uint64_t> cFpData::add64_carry(uint64_t a, uint64_t b, bool carry) const {
    bool carry_out;
    uint64_t sum;

    const uint64_t max = 0xFFFFFFFFFFFFFFFFULL;

    if ((a == max) && carry) {
        carry_out = 1;
    }
    else if ((a + carry) > (max - b)) {
        carry_out = 1;
    }
    else {
        carry_out = 0;
    }

    sum = a + b + carry;

    return std::make_tuple(carry_out, sum);

}

std::tuple<bool, uint32_t> cFpData::add32_carry(uint32_t a, uint32_t b, bool carry) const {
    bool carry_out;
    uint32_t sum;

    const uint32_t max = 0xFFFFFFFF;

    if ((a == max) && carry) {
        carry_out = 1;
    }
    else if ((a + carry) > (max - b)) {
        carry_out = 1;
    }
    else {
        carry_out = 0;
    }

    sum = a + b + carry;

    return std::make_tuple(carry_out, sum);

}

uint64_t cFpData::mul64_upper(uint64_t a, uint64_t b) const {
    uint64_t a_upper = a >> 32;
    uint64_t b_upper = b >> 32;

    uint64_t a_lower = a & 0xFFFFFFFF;
    uint64_t b_lower = b & 0xFFFFFFFF;

    uint64_t part0 = a_upper * b_upper;
    uint64_t part1 = a_upper * b_lower;
    uint64_t part2 = a_lower * b_upper;
    uint64_t part3 = a_lower * b_lower;

    bool carry;
    bool carry_mid;
    uint64_t sum;

    std::tie(carry, sum) = add64_carry(part3, (part1 + part2) << 32, 0);
    std::tie(carry_mid, sum) = add64_carry(part1, part2, 0);
    //std::tie(carry_mid, sum) = add64_carry(sum, part3 >> 32, carry);

    //sum = part0 + (sum >> 32) + ((uint64_t)carry_mid << 32);
    //printf("mul64_upper() carry:%0d\n", carry);
    //printf("mul64_upper() carry_mid:%0d\n", carry_mid);
    //printf("mul64_upper() part0:0x%llx\n", part0);
    if (carry_mid) {
        sum = part0 + (sum >> 32) + (1ULL << 32) + (uint64_t)carry;
    }
    else {
        sum = part0 + (sum >> 32) + (uint64_t)carry;
    }


    return sum;
}


uint64_t cFpData::mul64_lower(uint64_t a, uint64_t b) const {
    return a * b;

}


}
