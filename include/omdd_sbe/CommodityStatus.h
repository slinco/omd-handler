/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _OMDD_SBE_COMMODITYSTATUS_H_
#define _OMDD_SBE_COMMODITYSTATUS_H_

#if defined(SBE_HAVE_CMATH)
/* cmath needed for std::numeric_limits<double>::quiet_NaN() */
#  include <cmath>
#  define SBE_FLOAT_NAN std::numeric_limits<float>::quiet_NaN()
#  define SBE_DOUBLE_NAN std::numeric_limits<double>::quiet_NaN()
#else
/* math.h needed for NAN */
#  include <math.h>
#  define SBE_FLOAT_NAN NAN
#  define SBE_DOUBLE_NAN NAN
#endif

#if __cplusplus >= 201103L
#  include <cstdint>
#  include <string>
#  include <cstring>
#endif

#if __cplusplus >= 201103L
#  define SBE_CONSTEXPR constexpr
#  define SBE_NOEXCEPT noexcept
#else
#  define SBE_CONSTEXPR
#  define SBE_NOEXCEPT
#endif

#include <sbe/sbe.h>

#include "MessageHeader.h"
#include "GroupSize8Reverse.h"
#include "GroupSize8.h"

using namespace sbe;

namespace omdd {
namespace sbe {

class CommodityStatus
{
private:
    char *m_buffer;
    std::uint64_t m_bufferLength;
    std::uint64_t *m_positionPtr;
    std::uint64_t m_offset;
    std::uint64_t m_position;
    std::uint64_t m_actingBlockLength;
    std::uint64_t m_actingVersion;

    inline void reset(
        char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength, const std::uint64_t actingVersion)
    {
        m_buffer = buffer;
        m_offset = offset;
        m_bufferLength = bufferLength;
        m_actingBlockLength = actingBlockLength;
        m_actingVersion = actingVersion;
        m_positionPtr = &m_position;
        position(offset + m_actingBlockLength);
    }

    inline void reset(const CommodityStatus& codec)
    {
        m_buffer = codec.m_buffer;
        m_offset = codec.m_offset;
        m_bufferLength = codec.m_bufferLength;
        m_actingBlockLength = codec.m_actingBlockLength;
        m_actingVersion = codec.m_actingVersion;
        m_positionPtr = &m_position;
        m_position = codec.m_position;
    }

public:

    CommodityStatus() : m_buffer(nullptr), m_bufferLength(0), m_offset(0) {}

    CommodityStatus(char *buffer, const std::uint64_t bufferLength)
    {
        reset(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion());
    }

    CommodityStatus(char *buffer, const std::uint64_t bufferLength, const std::uint64_t actingBlockLength, const std::uint64_t actingVersion)
    {
        reset(buffer, 0, bufferLength, actingBlockLength, actingVersion);
    }

    CommodityStatus(const CommodityStatus& codec)
    {
        reset(codec);
    }

#if __cplusplus >= 201103L
    CommodityStatus(CommodityStatus&& codec)
    {
        reset(codec);
    }

    CommodityStatus& operator=(CommodityStatus&& codec)
    {
        reset(codec);
        return *this;
    }

#endif

    CommodityStatus& operator=(const CommodityStatus& codec)
    {
        reset(codec);
        return *this;
    }

    static SBE_CONSTEXPR std::uint16_t sbeBlockLength() SBE_NOEXCEPT
    {
        return (std::uint16_t)4;
    }

    static SBE_CONSTEXPR std::uint16_t sbeTemplateId() SBE_NOEXCEPT
    {
        return (std::uint16_t)322;
    }

    static SBE_CONSTEXPR std::uint16_t sbeSchemaId() SBE_NOEXCEPT
    {
        return (std::uint16_t)1;
    }

    static SBE_CONSTEXPR std::uint16_t sbeSchemaVersion() SBE_NOEXCEPT
    {
        return (std::uint16_t)1;
    }

    static SBE_CONSTEXPR const char * sbeSemanticType() SBE_NOEXCEPT
    {
        return "4";
    }

    std::uint64_t offset() const SBE_NOEXCEPT
    {
        return m_offset;
    }

    CommodityStatus &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
        reset(buffer, offset, bufferLength, sbeBlockLength(), sbeSchemaVersion());
        return *this;
    }

    CommodityStatus &wrapForDecode(
         char *buffer, const std::uint64_t offset, const std::uint64_t actingBlockLength,
         const std::uint64_t actingVersion, const std::uint64_t bufferLength)
    {
        reset(buffer, offset, bufferLength, actingBlockLength, actingVersion);
        return *this;
    }

    std::uint64_t position() const SBE_NOEXCEPT
    {
        return m_position;
    }

    void position(const std::uint64_t position)
    {
        if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false))
        {
            throw std::runtime_error("buffer too short [E100]");
        }
        m_position = position;
    }

    std::uint64_t encodedLength() const SBE_NOEXCEPT
    {
        return position() - m_offset;
    }

    char *buffer() SBE_NOEXCEPT
    {
        return m_buffer;
    }

    std::uint64_t bufferLength() const SBE_NOEXCEPT
    {
        return m_bufferLength;
    }

    std::uint64_t actingVersion() const SBE_NOEXCEPT
    {
        return m_actingVersion;
    }

    static SBE_CONSTEXPR std::uint16_t commodityCodeId() SBE_NOEXCEPT
    {
        return 101;
    }

    static SBE_CONSTEXPR std::uint64_t commodityCodeSinceVersion() SBE_NOEXCEPT
    {
         return 0;
    }

    bool commodityCodeInActingVersion() SBE_NOEXCEPT
    {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#endif
        return m_actingVersion >= commodityCodeSinceVersion();
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    }

    static SBE_CONSTEXPR std::size_t commodityCodeEncodingOffset() SBE_NOEXCEPT
    {
         return 0;
    }


    static const char *commodityCodeMetaAttribute(const MetaAttribute::Attribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::EPOCH: return "unix";
            case MetaAttribute::TIME_UNIT: return "nanosecond";
            case MetaAttribute::SEMANTIC_TYPE: return "";
            case MetaAttribute::PRESENCE: return "required";
        }

        return "";
    }

    static SBE_CONSTEXPR std::uint16_t commodityCodeNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT16;
    }

    static SBE_CONSTEXPR std::uint16_t commodityCodeMinValue() SBE_NOEXCEPT
    {
        return (std::uint16_t)0;
    }

    static SBE_CONSTEXPR std::uint16_t commodityCodeMaxValue() SBE_NOEXCEPT
    {
        return (std::uint16_t)65534;
    }

    static SBE_CONSTEXPR std::size_t commodityCodeEncodingLength() SBE_NOEXCEPT
    {
        return 2;
    }

    std::uint16_t commodityCode() const
    {
        return SBE_LITTLE_ENDIAN_ENCODE_16(*((std::uint16_t *)(m_buffer + m_offset + 0)));
    }

    CommodityStatus &commodityCode(const std::uint16_t value)
    {
        *((std::uint16_t *)(m_buffer + m_offset + 0)) = SBE_LITTLE_ENDIAN_ENCODE_16(value);
        return *this;
    }

    static SBE_CONSTEXPR std::uint16_t suspendedId() SBE_NOEXCEPT
    {
        return 102;
    }

    static SBE_CONSTEXPR std::uint64_t suspendedSinceVersion() SBE_NOEXCEPT
    {
         return 0;
    }

    bool suspendedInActingVersion() SBE_NOEXCEPT
    {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#endif
        return m_actingVersion >= suspendedSinceVersion();
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    }

    static SBE_CONSTEXPR std::size_t suspendedEncodingOffset() SBE_NOEXCEPT
    {
         return 2;
    }


    static const char *suspendedMetaAttribute(const MetaAttribute::Attribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::EPOCH: return "unix";
            case MetaAttribute::TIME_UNIT: return "nanosecond";
            case MetaAttribute::SEMANTIC_TYPE: return "";
            case MetaAttribute::PRESENCE: return "required";
        }

        return "";
    }

    static SBE_CONSTEXPR char suspendedNullValue() SBE_NOEXCEPT
    {
        return (char)0;
    }

    static SBE_CONSTEXPR char suspendedMinValue() SBE_NOEXCEPT
    {
        return (char)32;
    }

    static SBE_CONSTEXPR char suspendedMaxValue() SBE_NOEXCEPT
    {
        return (char)126;
    }

    static SBE_CONSTEXPR std::size_t suspendedEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    char suspended() const
    {
        return (*((char *)(m_buffer + m_offset + 2)));
    }

    CommodityStatus &suspended(const char value)
    {
        *((char *)(m_buffer + m_offset + 2)) = (value);
        return *this;
    }

    static SBE_CONSTEXPR std::uint16_t lockedId() SBE_NOEXCEPT
    {
        return 103;
    }

    static SBE_CONSTEXPR std::uint64_t lockedSinceVersion() SBE_NOEXCEPT
    {
         return 0;
    }

    bool lockedInActingVersion() SBE_NOEXCEPT
    {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#endif
        return m_actingVersion >= lockedSinceVersion();
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    }

    static SBE_CONSTEXPR std::size_t lockedEncodingOffset() SBE_NOEXCEPT
    {
         return 3;
    }


    static const char *lockedMetaAttribute(const MetaAttribute::Attribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::EPOCH: return "unix";
            case MetaAttribute::TIME_UNIT: return "nanosecond";
            case MetaAttribute::SEMANTIC_TYPE: return "";
            case MetaAttribute::PRESENCE: return "required";
        }

        return "";
    }

    static SBE_CONSTEXPR std::uint8_t lockedNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t lockedMinValue() SBE_NOEXCEPT
    {
        return (std::uint8_t)0;
    }

    static SBE_CONSTEXPR std::uint8_t lockedMaxValue() SBE_NOEXCEPT
    {
        return (std::uint8_t)254;
    }

    static SBE_CONSTEXPR std::size_t lockedEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    std::uint8_t locked() const
    {
        return (*((std::uint8_t *)(m_buffer + m_offset + 3)));
    }

    CommodityStatus &locked(const std::uint8_t value)
    {
        *((std::uint8_t *)(m_buffer + m_offset + 3)) = (value);
        return *this;
    }
};
}
}
#endif
