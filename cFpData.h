#pragma once

#include <tuple>

namespace nFp {
    class cFpData {
    public:
        typedef enum {
            ROUND_NEAREST_EVEN,
            ROUND_NEAREST_ODD,
            ROUND_POS_INF,
            ROUND_NEG_INF,
            ROUND_TOWARDS_ZERO,
            ROUND_AWAY_ZERO
        } eRoundMode;

        typedef enum {
            DP_POS_MAX_NORMAL_CONST,
            DP_NEG_MAX_NORMAL_CONST,
            DP_POS_MIN_NORMAL_CONST,
            DP_NEG_MIN_NORMAL_CONST,
            DP_POS_MAX_DENORMAL_CONST,
            DP_NEG_MAX_DENORMAL_CONST,
            DP_POS_MIN_DENORMAL_CONST,
            DP_NEG_MIN_DENORMAL_CONST

        } eFpDataConst;

        static const uint32_t BIAS = 0;
        static const int32_t EXP_MAX = 0x7FFFFFFF;
        static const uint64_t UINT64_MSB_ONE = 0x8000000000000000ULL;
        static const uint64_t DP_EXP_MASK = 0x7FF0000000000000ULL;
        static const uint32_t DP_EXP_LSB = 52;
        static const uint32_t DP_GUARD_POSITION = DP_EXP_LSB;
        static const uint64_t DP_MANT_MASK = 0xFFFFFFFFFFFFFULL;
        static const int32_t  DP_EXP_BIAS = 0x3FF;
        static const uint32_t SP_EXP_MASK = 0x7F800000;
        static const uint32_t SP_EXP_LSB = 23;
        static const uint32_t SP_MANT_MASK = 0x7FFFFF;
        static const int32_t  SP_EXP_BIAS = 0x7F;
        static const uint32_t SP_GUARD_POSITION = SP_EXP_LSB;

        static cFpData GetFpDataConst(eFpDataConst fpDataConst) {
            if (fpDataConst == DP_POS_MAX_NORMAL_CONST) {
                return cFpData(0, 0x3FE, 0xFFFFFFFFFFFFF800ULL);
            }
            else if (fpDataConst == DP_NEG_MAX_NORMAL_CONST) {
                return cFpData(1, 0x3FE, 0xFFFFFFFFFFFFF800ULL);
            }
            else if (fpDataConst == DP_POS_MIN_NORMAL_CONST) {
                return cFpData(0, 1 - DP_EXP_BIAS, UINT64_MSB_ONE);
            }
            else if (fpDataConst == DP_NEG_MIN_NORMAL_CONST) {
                return cFpData(1, 1 - DP_EXP_BIAS, UINT64_MSB_ONE);
            }
            else if (fpDataConst == DP_POS_MAX_DENORMAL_CONST) {
                return cFpData(0, 0 - DP_EXP_BIAS, DP_MANT_MASK << 12);
            }
            else if (fpDataConst == DP_NEG_MAX_DENORMAL_CONST) {
                return cFpData(1, 0 - DP_EXP_BIAS, DP_MANT_MASK << 12);
            }
            else if (fpDataConst == DP_POS_MIN_DENORMAL_CONST) {
                cFpData tmpResult = cFpData(0, 0 - DP_EXP_BIAS, 0x1ULL << 12);
                tmpResult.Normalize();
                return tmpResult;
            }
            else if (fpDataConst == DP_NEG_MIN_DENORMAL_CONST) {
                cFpData tmpResult = cFpData(1, 0 - DP_EXP_BIAS, 0x1ULL << 12);
                tmpResult.Normalize();
                return tmpResult;
            }
        }

        // Member variables

        // format s exp mant mant_ext sticky
        // integer bit is MSB of mant
        // explicit sticky_bit
        // Can do HP, SP, DP math
        // Not capable doing X87 extended precision math, need to add explicit round_bit to do it
        bool sign;
        int32_t exp;
        uint64_t mant;
        uint64_t mant_ext;
        bool sticky;
        eRoundMode round_mode;

        std::tuple<bool, uint64_t> add64_carry(uint64_t a, uint64_t b, bool carry) const;
        std::tuple<bool, uint32_t> add32_carry(uint32_t a, uint32_t b, bool carry) const;
        uint64_t mul64_upper(uint64_t a, uint64_t b) const;
        uint64_t mul64_lower(uint64_t a, uint64_t b) const;


        // Print
        void Print();

        // Constructor
        cFpData();
        cFpData(double number);
        cFpData(float number);
        cFpData(uint64_t number);
        cFpData(uint32_t number);
        cFpData(int64_t number);
        cFpData(int32_t number);
        cFpData(bool sign, int32_t exp, uint64_t mant, uint64_t mant_ext = 0, bool sticky = false);

        // Type convert
        double to_double() const;
        float to_float() const;

        // Operator override
        cFpData& operator=(const cFpData& fpData);
        bool operator>(const cFpData& fpData) const;
        bool operator<(const cFpData& fpData) const;
        bool operator==(const cFpData& fpData) const;
        bool operator!=(const cFpData& fpData) const;
        bool operator>=(const cFpData& fpData) const;
        bool operator<=(const cFpData& fpData) const;
        cFpData operator+(const cFpData& fpData) const;
        cFpData operator-(const cFpData& fpData) const;
        cFpData operator*(const cFpData& fpData) const;
        cFpData operator/(const cFpData& fpData) const;

        // Category
        bool isPos() const;
        bool isNeg() const;
        bool isNan() const;
        bool isInf() const;
        bool isZero() const;
        bool isDenormal() const;
        bool isNumerical() const;
        bool isMantMsbOne() const;
        bool isNormalized() const;

        // Operation
        cFpData addCore(const cFpData& operand0, const cFpData& operand1) const;
        cFpData mulCore(const cFpData& operand0, const cFpData& operand1) const;

        // No rounding performed at the end of the operation
        cFpData add(const cFpData& fpData) const; // 127-bit(source)(include integer bit) precision add 
        cFpData sub(const cFpData&) const; // same as add
        cFpData mul(const cFpData& fpData) const; // 63-bit(source)(include integer bit) precision mul (source mant_ext and sticky must be 0)
        cFpData div(const cFpData& fpData) const; // 63-bit(source)(include integer bit) precision div (source mant_ext and sticky must be 0), 
                                                  // Note: Current div implementation return 64-bit mant + 1bit sticky, if round guard_position need to < 63
        cFpData abs() const;
        cFpData neg() const;


        // Compare
        bool isGreater(const cFpData& fpData) const;
        bool isSmaller(const cFpData& fpData) const;
        bool isEqual(const cFpData& fpData) const;
        bool isGreaterEqual(const cFpData& fpData) const;
        bool isSmallerEqual(const cFpData& fpData) const;


        // Utilities
        bool getMantLsb() const;
        bool getMantExtLsb() const;
        bool isMantExtMsbOne() const;


        // Manipulate
        void Normalize();
        void IncrExpRightShiftMant();
        void DecExpLeftShiftMant();

        void SetInf(bool sign = 0);
        void SetZero(bool sign = 0);
        void SetSNan(bool sign = 0);
        void SetQNan(bool sign = 0);

        void Round(uint32_t guard_position); // guard_postion > 0 && guard_position < 64
        bool roundCore(bool sign, bool guard, bool round, bool sticky, eRoundMode rmode);
        void RoundDP();
        void RoundSP();

    };


}