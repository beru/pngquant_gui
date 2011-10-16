#pragma once

template <typename T>
bool InArray(const T& val, const T arr[])
{
	for (size_t i=0; i<ARRAY_SIZE(arr); ++i)
		if (*it == val)
			return true;
	return false;
}

/*
template <typename T>
inline
size_t Count(const T& arr)
{
	const size_t arrSize = sizeof(arr);
	const size_t elemSize = sizeof(arr[0]);
	return arrSize / elemSize;
}
*/
// #define  ARRAY_SIZE(arr)  (sizeof(arr)/sizeof(arr[0]))

template <typename T, size_t N>
size_t countof( T (&array)[N] )
{
    return N;
}

template <typename T>
__forceinline
T* OffsetPtr(T*& ptr, int offsetBytes)
{
	ptr = (T*) ((const char*)ptr + offsetBytes);
	return ptr;
}

#include <assert.h>

/*!
	�ړ����ϖ@�ŁA�z��̒l�����炩�ɂ���B����P�_�̗v�f�Ƃ��̑O��̗v�f�̒l�𕽋ω�����B
	@param srcArr				[in] �ޗ��Ƃ���z��B�v�f��arrSize�B
	@param resultArr			[out] ���炩�ɂ������ʂ��������ޔz��B�v�f��arrSize�B
	@param arrSize				[in] �z��̗v�f��
	@param smoothSampleBack		[in] �O�̗v�f����T���v�����鐔
	@param smoothSampleFront	[in] ��̗v�f����T���v�����鐔
*/
template <typename T>
void MovingAverage
(
	const T* srcArr,
	T* resultArr,
	size_t arrSize,
	size_t smoothSampleBack,
	size_t smoothSampleFront)
{
    // �O���sample�������݂���z��̗v�f���𒴂��Ă����ꍇ�A�[�̒l�����������z���p�ӂ��Čv�Z�ɗ��p����B
	// ���ׁ̈A�O�[�A�{�����A��[�A�Ə����𕪂���B

	size_t sampleCount;
	T* tmpArr;

	/// �O�[����
	// src����
	sampleCount = smoothSampleBack * 2 + smoothSampleFront;
	assert(sampleCount < arrSize);
	tmpArr = new T[sampleCount];
	std::fill(	// �O������
		&tmpArr[0],
		&tmpArr[smoothSampleBack],
		srcArr[0]
	);
	std::copy(	// �㔼copy
		&srcArr[smoothSampleBack],
		&srcArr[sampleCount],
		&tmpArr[smoothSampleBack]
	);
	MovingAverage_Routine(
		&tmpArr[0],
		&resultArr[0],
		smoothSampleBack,
		smoothSampleBack,
		smoothSampleFront
	);
	delete tmpArr;
 
	/// �{��������
	MovingAverage_Routine(
		&srcArr[0],
		&resultArr[smoothSampleBack],
		arrSize-smoothSampleFront-smoothSampleBack,
		smoothSampleBack,
		smoothSampleFront
	);

	/// ��[����
	// src����
	sampleCount = smoothSampleBack + smoothSampleFront * 2;
	assert(sampleCount < arrSize);
	tmpArr = new T[sampleCount];
	std::copy(	// �O��copy
		&srcArr[arrSize-smoothSampleFront],
		&srcArr[arrSize],
		&tmpArr[0]
	);
	std::fill(	// �㔼����
		&tmpArr[smoothSampleFront],
		&tmpArr[sampleCount],
		srcArr[arrSize-1]
	);
	MovingAverage_Routine(
		&tmpArr[0],
		&resultArr[arrSize-smoothSampleFront],
		smoothSampleFront,
		smoothSampleBack,
		smoothSampleFront
	);
	delete tmpArr;
}

/*!
	�ړ����ϖ@�ŁA�z��̒l�����炩�ɂ���routine�B����P�_�̗v�f�Ƃ��̑O��̗v�f�̒l�𕽋ω�����B
	@param srcArr				[in] �ޗ��Ƃ���z��B�v�f��arrSize + smoothSampleBack + smoothSampleFront
	@param resultArr			[out] ���炩�ɂ������ʂ��������ޔz��B�v�f��arrSize�B
	@param arrSize				[in] �z��̗v�f��
	@param smoothSampleBack		[in] �O�̗v�f����T���v�����鐔
	@param smoothSampleFront	[in] ��̗v�f����T���v�����鐔
*/
template <typename T>
static inline
void MovingAverage_Routine
(
	const T* srcArr,
	T* resultArr,
	size_t arrSize,
	size_t smoothSampleBack,
	size_t smoothSampleFront)
{
	size_t sampleCount = smoothSampleBack + 1 + smoothSampleFront;
	// �{��������
	T sum;
	for (size_t i=0; i<arrSize; ++i)
	{
		sum = 0;
		for (size_t idx=i; idx<i+sampleCount; ++idx)
		{
			sum += srcArr[idx];
		}
		resultArr[i] = sum / sampleCount;
	}
}

template <typename T, typename TARRAY>
bool InArray(const T& val, const TARRAY& arr)
{
	for (size_t i=0; i<ARRAY_SIZE(arr); ++i)
		if (arr[i] == val)
			return true;
	return false;
}

inline unsigned char ExtractByteFromBits(char* buff, size_t distanceBits)
{
	size_t distanceBytes = distanceBits / 8;
	size_t remainBits = distanceBits % 8;
	if (remainBits == 0) {
		return *(unsigned char*) (buff + distanceBytes);
	}else {
		unsigned char* v1 = (unsigned char*) (buff + distanceBytes);
		unsigned char* v2 = v1 + 1;
		return (*v1 << remainBits) | (*v2 >> (8 - remainBits));
	}
}

template <typename T>
T ExtractFromBits(char* buff, size_t distanceBits)
{
	size_t distanceBytes = distanceBits / 8;
	size_t remainBits = distanceBits % 8;
	if (remainBits == 0) {
		return *(T*) (buff + distanceBytes);
	}else {
		T* pv1 = (T*) (buff + distanceBytes);
		T v1 = *pv1;
		v1 = ConvertEndian(v1);
		v1 = (v1 << remainBits);
		v1 = ConvertEndian(v1);

		T* pv2 = pv1 + 1;
		T v2 = *pv2;
		v2 = ConvertEndian(v2);
		v2 = (v2 >> (sizeof(T)*8 - remainBits));
		v2 = ConvertEndian(v2);

		return v1 | v2;
	}
}

