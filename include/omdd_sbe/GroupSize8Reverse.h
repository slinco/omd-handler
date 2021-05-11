/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _OMDD_SBE_GROUPSIZE8REVERSE_H_
#define _OMDD_SBE_GROUPSIZE8REVERSE_H_

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


using namespace sbe;

namespace omdd {
namespace sbe {

class GroupSize8Reverse
{
private:
    char *m_buffer;
    std::uint64_t m_bufferLength;
    std::uint64_t m_offset;
    std::uint64_t m_actingVersion;

    inline void reset(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength, const std::uint64_t actingVersion)
    {
        if (SBE_BOUNDS_CHECK_EXPECT(((offset + 4) > bufferLength), false))
        {
            throw std::runtime_error("buffer too short for flyweight [E107]");
        }

        m_buffer = buffer;
        m_bufferLength = bufferLength;
        m_offset = offset;
        m_actingVersion = actingVersion;
    }

public:
    GroupSize8Reverse() : m_buffer(nullptr), m_offset(0) {}

    GroupSize8Reverse(char *buffer, const std::uint64_t bufferLength, const std::uint64_t actingVersion)
    {
        reset(buffer, 0, bufferLength, actingVersion);
    }

    GroupSize8Reverse(const GroupSize8Reverse& codec) :
        m_buffer(codec.m_buffer),
        m_bufferLength(codec.m_bufferLength),
        m_offset(codec.m_offset),
        m_actingVersion(codec.m_actingVersion){}

#if __cplusplus >= 201103L
    GroupSize8Reverse(GroupSize8Reverse&& codec) :
        m_buffer(codec.m_buffer),
        m_bufferLength(codec.m_bufferLength),
        m_offset(codec.m_offset),
        m_actingVersion(codec.m_actingVersion){}

    GroupSize8Reverse& operator=(GroupSize8Reverse&& codec) SBE_NOEXCEPT
    {
        m_buffer = codec.m_buffer;
        m_bufferLength = codec.m_bufferLength;
        m_offset = codec.m_offset;
        m_actingVersion = codec.m_actingVersion;
        return *this;
    }

#endif

    GroupSize8Reverse& operator=(const GroupSize8Reverse& codec) SBE_NOEXCEPT
    {
        m_buffer = codec.m_buffer;
        m_bufferLength = codec.m_bufferLength;
        m_offset = codec.m_offset;
        m_actingVersion = codec.m_actingVersion;
        return *this;
    }

    GroupSize8Reverse &wrap(char *buffer, const std::uint64_t offset, const std::uint64_t actingVersion, const std::uint64_t bufferLength)
    {
        reset(buffer, offset, bufferLength, actingVersion);
        return *this;
    }

    static SBE_CONSTEXPR std::uint64_t encodedLength() SBE_NOEXCEPT
    {
        return 4;
    }

    std::uint64_t offset() const SBE_NOEXCEPT
    {
        return m_offset;
    }

    char *buffer() SBE_NOEXCEPT
    {
        return m_buffer;
    }

    std::uint64_t bufferLength() const SBE_NOEXCEPT
    {
        return m_bufferLength;
    }


    static SBE_CONSTEXPR std::uint8_t filler1NullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t filler1MinValue() SBE_NOEXCEPT
    {
        return (std::uint8_t)0;
    }

    static SBE_CONSTEXPR std::uint8_t filler1MaxValue() SBE_NOEXCEPT
    {
        return (std::uint8_t)254;
    }

    static SBE_CONSTEXPR std::size_t filler1EncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    std::uint8_t filler1() const
    {
        return (*((std::uint8_t *)(m_buffer + m_offset + 0)));
    }

    GroupSize8Reverse &filler1(const std::uint8_t value)
    {
        *((std::uint8_t *)(m_buffer + m_offset + 0)) = (value);
        return *this;
    }

    static SBE_CONSTEXPR std::uint16_t blockLengthNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT16;
    }

    static SBE_CONSTEXPR std::uint16_t blockLengthMinValue() SBE_NOEXCEPT
    {
        return (std::uint16_t)0;
    }

    static SBE_CONSTEXPR std::uint16_t blockLengthMaxValue() SBE_NOEXCEPT
    {
        return (std::uint16_t)65534;
    }

    static SBE_CONSTEXPR std::size_t blockLengthEncodingLength() SBE_NOEXCEPT
    {
        return 2;
    }

    std::uint16_t blockLength() const
    {
        return SBE_LITTLE_ENDIAN_ENCODE_16(*((std::uint16_t *)(m_buffer + m_offset + 1)));
    }

    GroupSize8Reverse &blockLength(const std::uint16_t value)
    {
        *((std::uint16_t *)(m_buffer + m_offset + 1)) = SBE_LITTLE_ENDIAN_ENCODE_16(value);
        return *this;
    }

    static SBE_CONSTEXPR std::uint8_t numInGroupNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_UINT8;
    }

    static SBE_CONSTEXPR std::uint8_t numInGroupMinValue() SBE_NOEXCEPT
    {
        return (std::uint8_t)0;
    }

    static SBE_CONSTEXPR std::uint8_t numInGroupMaxValue() SBE_NOEXCEPT
    {
        return (std::uint8_t)254;
    }

    static SBE_CONSTEXPR std::size_t numInGroupEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    std::uint8_t numInGroup() const
    {
        return (*((std::uint8_t *)(m_buffer + m_offset + 3)));
    }

    GroupSize8Reverse &numInGroup(const std::uint8_t value)
    {
        *((std::uint8_t *)(m_buffer + m_offset + 3)) = (value);
        return *this;
    }
};
}
}
#endif
