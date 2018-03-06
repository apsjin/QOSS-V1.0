
/**************************************************************************************
* version 1.0 2016/11/7
* Designer liyun
* E-mail 465120258@qq.com
* Fuction�� 
* ����б�洫��
* ���� Ex Ey �õ�Ŀ�����Ex Ey Ez Hx Hy Hz
* ��Ҫ���������Ƶ�ʣ�Ĭ��10.65e9����Ŀ����롢�������N*M��
Ŀ������б�ǣ�x�᷽�򣩡�ds��Ĭ�ϲ���/3.5��
***************************************************************************************/
#pragma once
#ifndef CALCUlATION_H
#define CALCULATION_H

#include <cmath>
#include <complex>
#include "../util/GEMS_Memory.h"
#include "../util/Constant_Var.h"
#include "FFT.h"

class FFTDI
{
public:
	FFTDI(double f = 10.65e9, double z0 = 1,int N = 361, int M = 361);
	~FFTDI();

	void Initialization(); 
	void FreeCal();
	void Setds(double ds1); //����ds

	//�������벢��0
	void SetInput(std::complex<double> ** ExIn, std::complex<double> ** EyIn);

	void StartCal();

	//���Ex Ey Ez Hx Hy Hz ������FreeCal
	void output(std::complex<double> ** Ex, std::complex<double> ** Ey, std::complex<double> ** Ez, 
		std::complex<double> ** Hx, std::complex<double> ** Hy, std::complex<double> ** Hz );

private:
	//���帴���ĳ�
	void MulComp(const double r1, const double i1, const double r2, const double i2, double & outR, double & outI);
	std::complex<double> MulComp(std::complex<double> & In, double r2, double i2);
	//��ֵ����
	double InserVal(const double x0, const double yUp, const double y0, const double yDown);

	//���㴫�ﺯ��
	void Calh0();

	//�����볡������ת
	void Rotate();

	//�����볡���в�ֵ
	//void InserExEy();

private:
	double f; // Ƶ�� Ĭ�� 10.65e9
	double lamda; // ����
	double k;
	//double theta;
	//double theta_h0;
	double ds;
	double z0; //Ŀ�����

	int N, M; //����ĵ��� һ����Ϊ���� Ĭ��360
	int N2, M2; // N2 = 2 * N -1

	//���ݺ���
	std::complex<double> ** HExy, ** HEz_x0, **HEz_y0;
	std::complex<double> ** HHx_x0, ** HHx_y0,
		** HHy_x0, ** HHy_y0,
		** HHz_x0, ** HHz_y0;

	//��0�������
	std::complex<double> ** Ex0, ** Ey0;

	// ��˹ά������
	double 	PowerHExy, PowerFFTHExy;
	double 	PowerHEz_x0, PowerFFTHEz_x0;
	double 	PowerHEz_y0, PowerFFTHEz_y0;
	double 	PowerHHx_x0, PowerFFTHHx_x0;
	double 	PowerHHx_y0, PowerFFTHHx_y0;
	double 	PowerHHy_x0, PowerFFTHHy_x0;
	double 	PowerHHy_y0, PowerFFTHHy_y0;
	double 	PowerHHz_x0, PowerFFTHHz_x0;
	double 	PowerHHz_y0, PowerFFTHHz_y0;

	//Ex1 Ey1 Ez1 Hx1 Hy1 Hz1
	std::complex<double> ** Ex1, ** Ey1, **Ez1;
	std::complex<double> ** Hx1, ** Hy1, **Hz1;
};



#endif // !CALCUlATION_H