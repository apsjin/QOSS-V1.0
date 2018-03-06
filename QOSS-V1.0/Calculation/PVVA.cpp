#include "pvva.h"
#include "Position3D.h"
#include "Matrix4D.h"
#include "Vector3D.h"
#include <cmath>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkDelaunay2D.h>

using namespace calculation;


PVVA::PVVA()
{
	Initialization();
}

PVVA::~PVVA()
{
	Free_2D(Px);
	Free_2D(Py);
	Free_2D(Pz);

	Free_2D(Plane);
	Free_2D(n_Plane);
	Free_2D(R_Plane);
	
	Free_2D(Ex1);
	Free_2D(Ey1);
	Free_2D(Ez1);
	Free_2D(Hx1);
	Free_2D(Hy1);
	Free_2D(Hz1);

	Free_2D(Ex_R);
	Free_2D(Ey_R);
	Free_2D(Ez_R);
}

void PVVA::Initialization()
{
	N = 0; M = 0;
	f = 10.65e9;
	lamda = 0; 
	//k = 0;
	ds = 0; z0 = 0;
	unit_factor = 1;

	Ex_In = NULL;
	Ey_In = NULL;

	Px = NULL;
	Py = NULL;
	Pz = NULL;

	Plane = NULL;
	n_Plane = NULL;
	R_Plane = NULL;
	Rn_Plane = NULL;

	Ex1 = NULL;
	Ey1 = NULL;
	Ez1 = NULL;
	Hx1 = NULL;;
	Hy1 = NULL;
	Hz1 = NULL;

	Ex_R = NULL;
	Ey_R = NULL;
	Ez_R = NULL;
}

void PVVA::setUnit(double factor)
{
	unit_factor = factor;
}

double PVVA::IntersectPlane(const Vector3 & orig, const Vector3 & dir,
	const Vector3 & Plane_org, const Vector3 & Plane_n, Vector3 & intersection)
{
	double temp1 = dir.Dot(Plane_n);
	if(temp1 == 0)
		return 0.0;
	double temp2 = Plane_org.Dot(Plane_n) - orig.Dot(Plane_n);
	double t = temp2 / temp1;
	intersection = orig + dir * t;
	if (t > 10)
	{
		double l = 0;
	}
	return t;

}

double PVVA::IntersectPoint(const Vector3 &orig, const Vector3 &dir,
	const Vector3 &v0, const Vector3 &E1, const Vector3 &E2, Vector3 &intersection)
{
	// P
	Vector3 P = dir.Cross(E2);

	// determinant
	double det = E1.Dot(P);

	double u, v, t;
	Vector3 T;
	T = orig - v0;

	// If determinant is near zero, ray lies in plane of triangle
	if (det < 10e-10 && det > -10e-10)
		return 0;

	u = T.Dot(P);
	double fInvDet = 1.0f / det;
	u *= fInvDet;

	// Q
	Vector3 Q = T.Cross(E1);

	v = dir.Dot(Q);
	v *= fInvDet;

	// Calculate t, scale parameters, ray intersects triangle
	t = E2.Dot(Q);
	t *= fInvDet;

	intersection = orig + dir * t;
	return t;
}

void PVVA::Poynting()
{
	double tempx, tempy, tempz;
	double absx, absy, absz;
	//double sum = 0;

	// Դ������ϵת������������ϵ��ֻ����ת��
	Vector3D RotateAsixSou(SourceGraphTrans.getRotate_x(), 
		SourceGraphTrans.getRotate_y(),
		SourceGraphTrans.getRotate_z());
	Matrix4D rotatMatrixSou = Matrix4D::getRotateMatrix(
		SourceGraphTrans.getRotate_theta(), RotateAsixSou);


	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			Px[i][j] = ConjugateMul(Ey1[i][j], Hz1[i][j]) - ConjugateMul(Ez1[i][j], Hy1[i][j]);
			Py[i][j] = ConjugateMul(Ez1[i][j], Hx1[i][j]) - ConjugateMul(Ex1[i][j], Hz1[i][j]);
			Pz[i][j] = ConjugateMul(Ex1[i][j], Hy1[i][j]) - ConjugateMul(Ey1[i][j], Hx1[i][j]);

			absx = Px[i][j].real();
			absy = Py[i][j].real();
			absz = Pz[i][j].real();

			n_Plane[i][j].set(absx, absy, absz);

			n_Plane[i][j] = rotatMatrixSou * n_Plane[i][j];
			n_Plane[i][j].Normalization();
		}
	}

	//cout << "prePower:  " << sum << endl;

	/*ofstream outfile1("Px.txt");
	ofstream outfile2("Py.txt");
	ofstream outfile3("Pz.txt");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			outfile1 << Px[i][j].real() << " " << Px[i][j].imag() << "\n";
			outfile2 << Py[i][j].real() << " " << Py[i][j].imag() << "\n";
			outfile3 << Pz[i][j].real() << " " << Pz[i][j].imag() << "\n";
		}
	}
	outfile1.close();
	outfile2.close();
	outfile3.close();*/
}

void PVVA::getPlane(Vector3 **& _org, Vector3 **& _n) const
{
	for (int i = 0; i < N; i++)
		for (int j = 0; j < M; j++)
		{
			_org[i][j] = Plane[i][j];
			_n[i][j] = n_Plane[i][j];
		}
}

void PVVA::updateSource_n(Vector3 new_n)
{
	new_n.Normalization();
	if (new_n.x != 0 || new_n.y != 0 || new_n.z != 1)
	{
		Vector3 rotate_axis = Vector3(0, 0, 1).Cross(new_n); // ��ת��
		double rotate_theta = acos(Vector3(0, 0, 1).Dot(new_n));
		rotate_theta = rotate_theta / Pi * 180;
		SourceGraphTrans.updateRotate(rotate_axis, rotate_theta);
	}
	else
	{
		SourceGraphTrans.updateRotate(Vector3(0, 0, 1), 0);
	}
}

void PVVA::Result(double dis)
{
	CalPlane(dis);
	
	ofstream outfilex1("Ex_M.txt");
	ofstream outfilex2("Ey_M.txt");
	ofstream outfilex3("Ez_M.txt");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			outfilex1 << Ex1[i][j].real() << " " << Ex1[i][j].imag() << "\n";
			outfilex2 << Ey1[i][j].real() << " " << Ey1[i][j].imag() << "\n";
			outfilex3 << Ez1[i][j].real() << " " << Ez1[i][j].imag() << "\n";
		}
	}
	outfilex1.close();
	outfilex2.close();
	outfilex3.close();
}

void PVVA::CalAmplitude()
{
	Vector3 tempA1, tempA2;
	Vector3 tempB1, tempB2;
	Vector3 tempC1, tempC2;
	Vector3 tempD1, tempD2;
	double tempcos1, tempcos2;
	double PreSquare, PostSquare;
	double tempratio;
	double LAE, TAE;
	n_Source.Normalization();
	//���Ĳ����ĸ�����
	for (int i = 1; i < N - 2; i++)
	{
		for (int j = 1; j < M - 2; j++)
		{
			// ����ǰͶӰ���
			tempA1 = Plane[i - 1][j];
			tempB1 = Plane[i][j + 1];
			tempC1 = Plane[i + 1][j];
			tempD1 = Plane[i][j - 1];

			tempcos1 = n_Plane[i][j].Dot(n_Source);
		
			PreSquare = tempcos1 * ds * ds * 2;
			LAE = pow((Ex1[i][j].real() * Ex1[i][j].real() + Ex1[i][j].imag() * Ex1[i][j].imag()
				+ Ey1[i][j].real() * Ey1[i][j].real() + Ey1[i][j].imag() * Ey1[i][j].imag()
				+ Ez1[i][j].real() * Ez1[i][j].real() + Ez1[i][j].imag() * Ez1[i][j].imag()), 0.5);
			// �����ͶӰ���
			tempA2 = R_Plane[i - 1][j];
			tempB2 = R_Plane[i][j + 1];
			tempC2 = R_Plane[i + 1][j];
			tempD2 = R_Plane[i][j - 1];
			tempcos2 = Rn_Plane[i][j].Dot(R_Source);

			TAE = pow((Ex_R[i][j].real() * Ex_R[i][j].real() + Ex_R[i][j].imag() * Ex_R[i][j].imag()
				+ Ey_R[i][j].real() * Ey_R[i][j].real() + Ey_R[i][j].imag() * Ey_R[i][j].imag()
				+ Ez_R[i][j].real() * Ez_R[i][j].real() + Ez_R[i][j].imag() * Ez_R[i][j].imag()), 0.5);

			//PostSquare = tempcos2 * 
			double SquareTest1 = CalSquare(tempA2, tempB2, tempC2, tempD2);
			double SquareTest2 = CalSquare(tempB2, tempC2, tempD2, tempA2);
			if (abs(SquareTest1 - SquareTest2) < 0.00000001)
				PostSquare = tempcos2 * SquareTest1;
			else
			{
				PostSquare = PreSquare;
			}
			if (PostSquare == 0)
				PostSquare = 1;
			if (TAE == 0)
				TAE = 1;
			tempratio = pow(fabs(PreSquare / PostSquare), 0.5) * LAE / TAE;

			Ex_R[i][j] = Ex_R[i][j] * tempratio;
			Ey_R[i][j] = Ey_R[i][j] * tempratio;
			Ez_R[i][j] = Ez_R[i][j] * tempratio;
		}
	}
	/*
	//�߽粿��������
	  //�ϱ߽�
	for (int i = 0; i < N - 2 ; i++)
	{
		// ����ǰͶӰ���
		if (i == 0)
		{
			tempA1 = Plane[i][0];
			tempA2 = R_Plane[i][0];
		}
		else
		{
			tempA1 = Plane[i - 1][0];
			tempA2 = R_Plane[i - 1][0];
		}

		tempC1 = Plane[i + 1][0];
		tempD1 = Plane[i][1];
		tempn1 = (tempD1 - Plane[i][0]).Cross(tempC1 - Plane[i][0]);
		tempcos1 = tempn1.Dot(n_Plane[i][0]) / tempn1.Length() / n_Plane[i][0].Length();

		LAE = pow((Ex1[i][0].real() * Ex1[i][0].real() + Ex1[i][0].imag() * Ex1[i][0].imag()
			+ Ey1[i][0].real() * Ey1[i][0].real() + Ey1[i][0].imag() * Ey1[i][0].imag()
			+ Ez1[i][0].real() * Ez1[i][0].real() + Ez1[i][0].imag() * Ez1[i][0].imag()), 0.5);

		//PreSquare = abs(tempcos1) * CalSquare(tempA1, tempC1, tempD1);
		PreSquare =  CalSquare(tempA1, tempC1, tempD1);

		// �����ͶӰ���
		tempC2 = R_Plane[i + 1][0];
		tempD2 = R_Plane[i][1];
		tempn2 = (tempD2 - R_Plane[i][0]).Cross(tempC2 - R_Plane[i][0]);
		tempcos2 = tempn2.Dot(R_Plane[i][0]) / tempn2.Length() / R_Plane[i][0].Length();

		TAE = pow((Ex_R[i][0].real() * Ex_R[i][0].real() + Ex_R[i][0].imag() * Ex_R[i][0].imag()
			+ Ey_R[i][0].real() * Ey_R[i][0].real() + Ey_R[i][0].imag() * Ey_R[i][0].imag()
			+ Ez_R[i][0].real() * Ez_R[i][0].real() + Ez_R[i][0].imag() * Ez_R[i][0].imag()), 0.5);

		//PostSquare = abs(tempcos2) * CalSquare(tempA2, tempC2, tempD2);
		PostSquare =  CalSquare(tempA2, tempC2, tempD2);
		if (PostSquare == 0)
			PostSquare = 1;
		if (TAE == 0)
			TAE = 1;
		tempratio = pow(abs(PreSquare / PostSquare), 0.5) * LAE / TAE;

		Ex_R[i][0] = Ex_R[i][0] * tempratio;
		Ey_R[i][0] = Ey_R[i][0] * tempratio;
		Ez_R[i][0] = Ez_R[i][0] * tempratio;
	}
	  //�±߽�
	for (int i = 1; i < N - 1; i++)
	{
		// ����ǰͶӰ���
		if (i == N - 1)
		{
			tempC1 = Plane[i][N - 1];
			tempC2 = R_Plane[i][N - 1];
		}
		else
		{
			tempC1 = Plane[i + 1][N - 1];
			tempC2 = R_Plane[i + 1][N - 1];
		}
		tempA1 = Plane[i - 1][N - 1];
		tempB1 = Plane[i][N - 2];
		
		tempn1 = (tempA1 - Plane[i][N - 1]).Cross(tempB1 - Plane[i][N - 1]);
		tempcos1 = tempn1.Dot(n_Plane[i][N - 1]) / tempn1.Length() / n_Plane[i][N - 1].Length();

		LAE = pow((Ex1[i][N - 1].real() * Ex1[i][N - 1].real() + Ex1[i][N - 1].imag() * Ex1[i][N - 1].imag()
			+ Ey1[i][N - 1].real() * Ey1[i][N - 1].real() + Ey1[i][N - 1].imag() * Ey1[i][N - 1].imag()
			+ Ez1[i][N - 1].real() * Ez1[i][N - 1].real() + Ez1[i][N - 1].imag() * Ez1[i][N - 1].imag()), 0.5);

		PreSquare = abs(tempcos1) * CalSquare(tempA1, tempB1, tempC1);

		// �����ͶӰ���
		tempA2 = R_Plane[i - 1][N - 1];
		tempB2 = R_Plane[i][N - 2];
		
		tempn2 = (tempA2 - R_Plane[i][N - 1]).Cross(tempB2 - R_Plane[i][N - 1]);
		if (R_Plane[i][N - 1].Length() == 0 || tempn2.Length() == 0)
			tempcos2 = 1;
		else
			tempcos2 = tempn2.Dot(R_Plane[i][N - 1]) / tempn2.Length() 
			/ R_Plane[i][N - 1].Length();


		TAE = pow((Ex_R[i][N - 1].real() * Ex_R[i][N - 1].real() + Ex_R[i][N - 1].imag() * Ex_R[i][N - 1].imag()
			+ Ey_R[i][N - 1].real() * Ey_R[i][N - 1].real() + Ey_R[i][N - 1].imag() * Ey_R[i][N - 1].imag()
			+ Ez_R[i][N - 1].real() * Ez_R[i][N - 1].real() + Ez_R[i][N - 1].imag() * Ez_R[i][N - 1].imag()), 0.5);

		PostSquare = abs(tempcos2) * CalSquare(tempA2, tempB2, tempC2);
		if (PostSquare == 0)
			PostSquare = 1;
		if (TAE == 0)
			TAE = 1;
		tempratio = pow(abs(PreSquare / PostSquare), 0.5) * LAE / TAE;

		Ex_R[i][N - 1] = Ex_R[i][N - 1] * tempratio;
		Ey_R[i][N - 1] = Ey_R[i][N - 1] * tempratio;
		Ez_R[i][N - 1] = Ez_R[i][N - 1] * tempratio;
	}
	  //��߽�
	for (int j = 1; j < N - 1; j++)
	{
		// ����ǰͶӰ���
		if (j == N - 1)
		{
			tempD1 = Plane[0][j];
			tempD2 = R_Plane[0][j];
		}
		else
		{
			tempD1 = Plane[0][j + 1];
			tempD2 = R_Plane[0][j + 1];
		}
		tempB1 = Plane[0][j - 1];
		tempC1 = Plane[1][j];
		
		tempn1 = (tempB1 - Plane[0][j]).Cross(tempC1 - Plane[0][j]);
		if (tempn1.Length() == 0 || n_Plane[0][j].Length())
			tempcos1 = 1;
		else 
			tempcos1 = tempn1.Dot(n_Plane[0][j]) / tempn1.Length()
			/ n_Plane[0][j].Length();

		LAE = pow((Ex1[0][j].real() * Ex1[0][j].real() + Ex1[0][j].imag() * Ex1[0][j].imag()
			+ Ey1[0][j].real() * Ey1[0][j].real() + Ey1[0][j].imag() * Ey1[0][j].imag()
			+ Ez1[0][j].real() * Ez1[0][j].real() + Ez1[0][j].imag() * Ez1[0][j].imag()), 0.5);

		//PreSquare = abs(tempcos1) * CalSquare(tempB1, tempC1, tempD1);
		PreSquare = CalSquare(tempB1, tempC1, tempD1);
		// �����ͶӰ���
		tempB2 = R_Plane[0][j - 1];
		tempC2 = R_Plane[1][j];
		
		tempn2 = (tempB2 - R_Plane[0][j]).Cross(tempC2 - R_Plane[0][j]);
		if (R_Plane[0][j].Length() == 0 || tempn2.Length() == 0)
			tempcos2 = 1;
		else
			tempcos2 = tempn2.Dot(R_Plane[0][j]) / tempn2.Length() / R_Plane[0][j].Length();


		TAE = pow((Ex_R[0][j].real() * Ex_R[0][j].real() + Ex_R[0][j].imag() * Ex_R[0][j].imag()
			+ Ey_R[0][j].real() * Ey_R[0][j].real() + Ey_R[0][j].imag() * Ey_R[0][j].imag()
			+ Ez_R[0][j].real() * Ez_R[0][j].real() + Ez_R[0][j].imag() * Ez_R[0][j].imag()), 0.5);

		//PostSquare = abs(tempcos2) * CalSquare(tempB2, tempC2, tempD2);
		PostSquare = CalSquare(tempB2, tempC2, tempD2);
		if (PostSquare == 0)
			PostSquare = 1;
		if (TAE == 0)
			TAE = 1;
		tempratio = pow(abs(PreSquare / PostSquare), 0.5) * LAE / TAE;

		Ex_R[0][j] = Ex_R[0][j] * tempratio;
		Ey_R[0][j] = Ey_R[0][j] * tempratio;
		Ez_R[0][j] = Ez_R[0][j] * tempratio;
	}
	  //�ұ߽�
	for (int j = 0; j < N - 2; j++)
	{
		// ����ǰͶӰ���
		if (j == 0)
		{
			tempB1 = Plane[N - 1][j];
			tempB2 = R_Plane[N - 1][j];
		}
		else
		{
			tempB1 = Plane[N - 1][j - 1];
			tempB2 = R_Plane[N - 1][j - 1];
		}
		tempA1 = Plane[N - 2][j];
		
		tempD1 = Plane[N - 1][j + 1];
		tempn1 = (tempD1 - Plane[N - 1][j]).Cross(tempA1 - Plane[N - 1][j]);
		tempcos1 = tempn1.Dot(n_Plane[N - 1][j]) / tempn1.Length() / n_Plane[N - 1][j].Length();

		LAE = pow((Ex1[N - 1][j].real() * Ex1[N - 1][j].real() + Ex1[N - 1][j].imag() * Ex1[N - 1][j].imag()
			+ Ey1[N - 1][j].real() * Ey1[N - 1][j].real() + Ey1[N - 1][j].imag() * Ey1[N - 1][j].imag()
			+ Ez1[N - 1][j].real() * Ez1[N - 1][j].real() + Ez1[N - 1][j].imag() * Ez1[N - 1][j].imag()), 0.5);

		//PreSquare = abs(tempcos1) * CalSquare(tempD1, tempA1, tempB1);
		PreSquare = CalSquare(tempD1, tempA1, tempB1);
		// �����ͶӰ���
		tempA2 = R_Plane[N - 2][j];
		
		tempD2 = R_Plane[N - 1][j + 1];
		tempn2 = (tempD2 - R_Plane[N - 1][j]).Cross(tempA2 - R_Plane[N - 1][j]);
		tempcos2 = tempn2.Dot(R_Plane[N - 1][j]) / tempn2.Length() / R_Plane[N - 1][j].Length();

		TAE = pow((Ex_R[N - 1][j].real() * Ex_R[N - 1][j].real() + Ex_R[N - 1][j].imag() * Ex_R[N - 1][j].imag()
			+ Ey_R[N - 1][j].real() * Ey_R[N - 1][j].real() + Ey_R[N - 1][j].imag() * Ey_R[N - 1][j].imag()
			+ Ez_R[N - 1][j].real() * Ez_R[N - 1][j].real() + Ez_R[N - 1][j].imag() * Ez_R[N - 1][j].imag()), 0.5);

		//PostSquare = abs(tempcos2) * CalSquare(tempD2, tempA2, tempB2
		PostSquare = CalSquare(tempD2, tempA2, tempB2);
		if (PostSquare == 0)
			PostSquare = 1;
		if (TAE == 0)
			TAE = 1;
		tempratio = pow(abs(PreSquare / PostSquare), 0.5) * LAE / TAE;

		Ex_R[N - 1][j] = Ex_R[N - 1][j] * tempratio;
		Ey_R[N - 1][j] = Ey_R[N - 1][j] * tempratio;
		Ez_R[N - 1][j] = Ez_R[N - 1][j] * tempratio;
	}*/
}

void PVVA::InterVal()
{
	// �����
	Vector3D RotateAsix(SourceGraphTrans.getRotate_x(),
		SourceGraphTrans.getRotate_y(),
		SourceGraphTrans.getRotate_z());
	Matrix4D R_rotatMatrix = Matrix4D::getRotateMatrix(
		-SourceGraphTrans.getRotate_theta(), RotateAsix);
	Matrix4D R_translateMatrix = Matrix4D::getTranslateMatrix(
		-SourceGraphTrans.getTrans_x(),
		-SourceGraphTrans.getTrans_y(), -SourceGraphTrans.getTrans_z());
	Matrix4D R_Matrix = R_rotatMatrix * R_translateMatrix;

	vtkFloatArray *scalars = vtkFloatArray::New();
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	double MAXX = ds * (N - 1) / 2;
	double MAXY = ds * (M - 1) / 2;
	int num = 0;
	for (int i = 0; i < N; ++i)
		for (int j = 0; j < M; ++j)
		{
			Ex_In[i][j] = complex<double>(0, 0);
			Ey_In[i][j] = complex<double>(0, 0);
			Position3D tempPoint((i - (N - 1) / 2) * ds,
				(j - (M - 1) / 2) * ds, 0);
			Plane[i][j].set(tempPoint.X(), tempPoint.Y(), tempPoint.Z());

			R_Plane[i][j] = R_Matrix * R_Plane[i][j];
			if (R_Plane[i][j].x < -MAXX || R_Plane[i][j].x > MAXX ||
				R_Plane[i][j].y < -MAXY || R_Plane[i][j].y > MAXY)
				continue;
			points->InsertNextPoint(R_Plane[i][j].x, R_Plane[i][j].y, 0);
			scalars->InsertTuple1(num++, i*N + j);
		}

	vtkSmartPointer<vtkPolyData> polydata =
		vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(points);
	polydata->GetPointData()->SetScalars(scalars);

	double scalarsRange[2];
	scalars->GetRange(scalarsRange);
	scalars->Delete();

	vtkSmartPointer<vtkDelaunay2D> delaunay =
		vtkSmartPointer<vtkDelaunay2D>::New();
	delaunay->SetInputData(polydata);
	delaunay->Update();
	polydata = delaunay->GetOutput();

	int EleNum = polydata->GetNumberOfCells();
	double t;
	for (int i = 0; i < EleNum; i++)  //���뷴����Ľ���
	{
		double maxX = -MAXX;
		double minX = MAXX;
		double maxY = -MAXY;
		double minY = MAXY;

		vtkIdList * p;
		p = polydata->GetCell(i)->GetPointIds();

		double * point;
		vector<Vector2> A(3);
		vector<int> dataIJ(3);
		Vector2 P;
		for (int k = 0; k < 3; k++)
		{
			dataIJ[k] = polydata->GetPointData()
				->GetScalars()->GetVariantValue(p->GetId(k)).ToInt();
			point = polydata->GetPoint(p->GetId(k));
			if (maxX < point[0])
				maxX = point[0];
			if (minX > point[0])
				minX = point[0];
			if (maxY < point[1])
				maxY = point[1];
			if (minY > point[1])
				minY = point[1];
			A[k].set(point[0], point[1]);
		}
		int ii = ceil(minX / ds) + (N - 1) / 2;
		int iiMax = floor(maxX / ds) + (N - 1) / 2;
		int jjMin = ceil(minY / ds) + (M - 1) / 2;
		int jjMax = floor(maxY / ds) + (M - 1) / 2;
		if (ii < 0)
			ii = 0;
		if (jjMin < 0)
			jjMin = 0;
		for (; ii <= iiMax&&ii<N; ii++)
			for (int jj = jjMin; jj <= jjMax&&jj<M; jj++)
			{
				P.set(Plane[ii][jj].x, Plane[ii][jj].y);
				if (InterVal_PointinTriangle(A[0], A[1], A[2], P))
				{
					double a = (P - A[0]).area();
					double b = (P - A[1]).area();
					double c = (P - A[2]).area();

					Ex_In[ii][jj] = (
						b * c * Ex_R[dataIJ[0] / N][dataIJ[0] % N] +
						a * c * Ex_R[dataIJ[1] / N][dataIJ[1] % N] +
						b * a * Ex_R[dataIJ[2] / N][dataIJ[2] % N]) /
						(b * c + a * c + b * a);
					Ey_In[ii][jj] = ( 
						b * c * Ey_R[dataIJ[0] / N][dataIJ[0] % N] + 
						a * c * Ey_R[dataIJ[1] / N][dataIJ[1] % N] +
						b * a * Ey_R[dataIJ[2] / N][dataIJ[2] % N]) /
						(b * c + a * c + b * a);
				}
			}
	}

	Matrix4D rotatMatrix = Matrix4D::getRotateMatrix(
		SourceGraphTrans.getRotate_theta(), RotateAsix);
	Matrix4D translateMatrix = Matrix4D::getTranslateMatrix(
		SourceGraphTrans.getTrans_x(),
		SourceGraphTrans.getTrans_y(), SourceGraphTrans.getTrans_z());

	// ����Դ�������λ��
	for (int i = 0; i < N; i++)
		for (int j = 0; j < M; j++)
		{
			Plane[i][j] = translateMatrix * rotatMatrix * Plane[i][j];
		}
	
	ofstream outfile1("Exy.txt");
	//ofstream outfile2("Ey_M.txt");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			outfile1 << abs(Ex_In[i][j]) << " " << arg(Ex_In[i][j]) / Pi *180 << " "
				<< abs(Ey_In[i][j]) << " " << arg(Ey_In[i][j]) / Pi * 180 << endl;
			//outfile1 << Ex_In[i][j].real() << " " << Ex_In[i][j].imag() << "\n";
			//outfile2 << Ey_In[i][j].real() << " " << Ey_In[i][j].imag() << "\n";
		}
	}
	outfile1.close();
	//outfile2.close();
}

void PVVA::CalReflectExyz(const Vector3 & n, const complex<double>& Ex,
	const complex<double>& Ey, const complex<double>& Ez, complex<double>& Ex_out,
	complex<double>& Ey_out, complex<double>& Ez_out)
{
	complex<double> ne(n.x * Ex.real() + n.y * Ey.real() + n.z * Ez.real(),
		n.x * Ex.imag() + n.y * Ey.imag() + n.z * Ez.imag());
	complex<double> tempx = 2 * n.x * ne;
	complex<double> tempy = 2 * n.y * ne;
	complex<double> tempz = 2 * n.z * ne;

	Ex_out = tempx - Ex;
	Ey_out = tempy - Ey;
	Ez_out = tempz - Ez;
}

double PVVA::CalSquare(const Vector3 & A, const Vector3 & B, const Vector3 & C, const Vector3 & D) const
{
	double AB = (B - A).Length();
	double AC = (C - A).Length();
	double AD = (D - A).Length();
	double BC = (C - B).Length();
	double DC = (C - D).Length();
	double p1 = (AB + AC + BC) / 2;
	double p2 = (AD + AC + DC) / 2;
	//Vector3 tempS1 = AB.Cross(AC);
	//Vector3 tempS2 = AC.Cross(AD);
	double tempS1 = sqrt(p1*(p1 - AB)*(p1 - BC)*(p1 - AC));
	double tempS2 = sqrt(p2*(p2 - AC)*(p2 - AD)*(p2 - DC));
	//double tempS = tempS1.Length() + tempS2.Length();
	return tempS1 + tempS2;
}

double PVVA::CalSquare(const Vector3 & A, const Vector3 & B,
	const Vector3 & C) const
{
	double AB = (B - A).Length();
	double AC = (C - A).Length();
	double BC = (B - C).Length();
	double p1 = (AB + AC + BC) / 2;
	
	return sqrt(p1*(p1 - AB)*(p1 - BC)*(p1 - AC));
}

bool PVVA::get_line_intersection(
	const Vector2 & A, const Vector2 & B, 
	const Vector2 & C, const Vector2 & D, Vector2 & O)
{
	double s10_x = B.x - A.x;
	double s10_y = B.y - A.y;
	double s32_x = D.x - C.x;
	double s32_y = D.y - C.y;

	double denom = s10_x * s32_y - s32_x * s10_y;
	if (abs(denom) < 0.000001)//ƽ�л���
		return 0; // Collinear
	bool denomPositive = denom > 0.0;

	double s02_x = A.x - C.x;
	double s02_y = A.y - C.y;
	double s_numer = s10_x * s02_y - s10_y * s02_x;
	if ((s_numer < 0.0) == denomPositive)
		//�����Ǵ��ڵ���0��С�ڵ���1�ģ����ӷ�ĸ����ͬ���ҷ���С�ڵ��ڷ�ĸ
		return false; // No collision

	double t_numer = s32_x * s02_y - s32_y * s02_x;
	if ((t_numer < 0.0) == denomPositive)
		return false; // No collision

	if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
		return false; // No collision
				  // Collision detected
	double t = t_numer / denom;
	
	O.set(A.x + (t * s10_x), A.y + (t * s10_y));

	return true;
}

void PVVA::AllocateMemory()
{
	//��ӡ͢ʸ��
	Px = Allocate_2D(Px, N, M);
	Py = Allocate_2D(Py, N, M);
	Pz = Allocate_2D(Pz, N, M);

	//Դ
	Ex_In = Allocate_2D(Ex_In, N, M);
	Ey_In = Allocate_2D(Ey_In, N, M);

	//������ĵ�ų�
	Ex1 = Allocate_2D(Ex1, N, M);
	Ey1 = Allocate_2D(Ey1, N, M);
	Ez1 = Allocate_2D(Ez1, N, M);

	Hx1 = Allocate_2D(Hx1, N, M);
	Hy1 = Allocate_2D(Hy1, N, M);
	Hz1 = Allocate_2D(Hz1, N, M);

	//�����ĵ糡
	Ex_R = Allocate_2D(Ex_R, N, M);
	Ey_R = Allocate_2D(Ey_R, N, M);
	Ez_R = Allocate_2D(Ez_R, N, M);

	// ƽ��λ�ã������ȣ�����󣩺͸���ķ�����
	n_Plane = Allocate_2D(n_Plane, N, M);
	Plane = Allocate_2D(Plane, N, M);
	R_Plane = Allocate_2D(R_Plane, N, M);
	Rn_Plane = Allocate_2D(Rn_Plane, N, M);

}

void PVVA::setSource(const Field* _field)
{
	const vector<vector<complex<double>>>&Ex_temp = _field->getEx();
	const vector<vector<complex<double>>>&Ey_temp = _field->getEy();

	_field->getSourcePara(SourceGraphTrans, N, M, ds);
	SourceGraphTrans.normalization(unit_factor);
	ds = ds * unit_factor;

	AllocateMemory();

	for(int i = 0; i < N; i++)
		for (int j = 0; j < M; j++)
		{
			Ex_In[i][j] = Ex_temp[i][j];
			Ey_In[i][j] = Ey_temp[i][j];
		}

	Vector3D RotateAsix(SourceGraphTrans.getRotate_x(), SourceGraphTrans.getRotate_y(),
		SourceGraphTrans.getRotate_z());
	Matrix4D rotatMatrix = Matrix4D::getRotateMatrix(SourceGraphTrans.getRotate_theta(), RotateAsix);
	Matrix4D translateMatrix = Matrix4D::getTranslateMatrix(SourceGraphTrans.getTrans_x(),
		SourceGraphTrans.getTrans_y(), SourceGraphTrans.getTrans_z());


	// ����Դ�������λ��
	for (int i = 0; i < N; i++)
		for (int j = 0; j < M; j++)
		{
			Position3D tempPoint((i - (N - 1) / 2) * ds, (j - (M - 1) / 2) * ds, 0);
			tempPoint = translateMatrix * rotatMatrix * tempPoint;
			Plane[i][j].set(tempPoint.X(), tempPoint.Y(), tempPoint.Z());
		}

	// ����Դ�ķ���
	Position3D tempPoint(0, 0, 1);
	tempPoint = rotatMatrix * tempPoint;
	n_Source.set(tempPoint.X(), tempPoint.Y(), tempPoint.Z());
	n_Source.Normalization();
	
	// ƽ����е�
	Position3D tempPoint1(0, 0, 0);
	tempPoint1 = translateMatrix * rotatMatrix * tempPoint1;
	Org_Source.set(tempPoint1.X(), tempPoint1.Y(), tempPoint1.Z());	

//	updateSource_n(Vector3(1, 1, 0));
}

void PVVA::getSourcePoint(Vector3 & interPoint, Vector3 & n_Point) const
{
	interPoint = Org_Source;
	n_Point = n_Source;
}

void PVVA::setSourcePoint(const Vector3 & interPoint, const Vector3 & n_Point)
{
	Org_Source = interPoint;
	n_Source = n_Point;
}

GraphTrans PVVA::getSourceGraphTrans(const Vector3 & n_Point)
{
	Vector3 a = n_Point;
	GraphTrans res;
	a.Normalization();
	if (a.x != 0 || a.y != 0 || a.z != 1)
	{
		Vector3 rotate_axis = Vector3(0, 0, 1).Cross(a); // ��ת��
		double rotate_theta = acos(Vector3(0, 0, 1).Dot(a));
		rotate_theta = rotate_theta / Pi * 180;
	
		res.updateRotate(rotate_axis, rotate_theta);
	}
	else
	{
		res.updateRotate(Vector3(0, 0, 1), 0);
	}
	return res;
}

void PVVA::setFre(double _fre)
{
	f = _fre;
	lamda = C_Speed / f;
	//k = 2 * Pi * f / C_Speed;
}

void PVVA::setMirror(Mirror * mirror)
{
	this->mirror = mirror;
}

bool PVVA::CheckModle(Vector3 & interPoint, Vector3 & n_Point)
{
	return false;
}

void PVVA::getField(Field * _field)
{
	_field->setNM(N, M);
	_field->setPlane(SourceGraphTrans, ds);
	_field->setField(Ex1, Ey1, Ez1, Hx1, Hy1, Hz1);
	_field->setShowPara(1, 1, 0);
	_field->updateData();
}

void PVVA::CalZ0Theta()
{
	Vector3 InterPoint, tempReflect;
	double temp;
	int i = 0;
	RayTracing rayTracing(mirror);
	bool tempIsIntersect = false;
	rayTracing.calcReflect(Org_Source, n_Source, tempReflect,
		InterPoint, tempIsIntersect);
	if (!tempIsIntersect)
		return; // û���ཻ

	//����ƽ�Ƶľ��루��ʱ��������û�иı䣩
	z0 = CalDistance(Org_Source, InterPoint);

	CalPlane(z0);

	R_Source = tempReflect;
	R_Source.Normalization();
	Poynting();
	//z0 = 0.525;
	//cout << "Distance of moving " << z0 << endl;
}

void PVVA::CalPlane(double dis)
{
	//cout << "Calculating plane wave propagation " << endl;
	//theta = Pi / 12;
	//double theta1 = Pi / 6;
	FFTDI cal(f, dis, N, M);
	cal.Setds(ds);
	cal.SetInput(Ex_In, Ey_In);
	
	cal.StartCal();
	cal.output(Ex1, Ey1, Ez1, Hx1, Hy1, Hz1); // ����ƽ�洫��

	for (int i = 0; i < N; i++)     // ����ƽ��λ��
		for (int j = 0; j < M; j++)
		{
			Plane[i][j] = Plane[i][j] + n_Source * dis;
		}

	Org_Source = Org_Source + n_Source * dis;
	SourceGraphTrans.updateTranslate(Org_Source);
	//n_Source.set(0, 0, -1);

	/*
	ofstream outfilex1("Ex1.txt");
	ofstream outfilex2("Ey1.txt");
	ofstream outfilex3("Ez1.txt");
	ofstream outfilex4("Hx1.txt");
	ofstream outfilex5("Hy1.txt");
	ofstream outfilex6("Hz1.txt");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			outfilex1 << Ex1[i][j].real() << " " << Ex1[i][j].imag() << "\n";
			outfilex2 << Ey1[i][j].real() << " " << Ey1[i][j].imag() << "\n";
			outfilex3 << Ez1[i][j].real() << " " << Ez1[i][j].imag() << "\n";
			outfilex4 << Hx1[i][j].real() << " " << Hx1[i][j].imag() << "\n";
			outfilex5 << Hy1[i][j].real() << " " << Hy1[i][j].imag() << "\n";
			outfilex6 << Hz1[i][j].real() << " " << Hz1[i][j].imag() << "\n";
		}
		
	}
	outfilex1.close();
	outfilex2.close();
	outfilex3.close();
	outfilex4.close();
	outfilex5.close();
	outfilex6.close();*/
	
	//cout << "Calculating plane wave propagation Completed" << endl;
}

void PVVA::Reflect()  
{
	//cout << "Calculating Reflecting surface" << endl;
	
	// ����ÿһ���� N * M * EleNum 
	Vector3 InterPoint;
	Vector3 Reflight, n_light; // ������� �� ������
	Vector3 n_light_Plane;  // �����ƽ��ķ�����

	// ��������ϵת����Դ������ϵ��ֻ����ת��
	Vector3D RotateAsixSou(SourceGraphTrans.getRotate_x(), SourceGraphTrans.getRotate_y(),
		SourceGraphTrans.getRotate_z());
	Matrix4D rotatMatrixSou = Matrix4D::getRotateMatrix(-SourceGraphTrans.getRotate_theta(), RotateAsixSou);

	Vector3 tempa, tempb;

	double tempx, tempz, tempy;  // ����ת����ʱ����
	double dir_t;  // ��ӡ͢ʸ�������ϵ��
	double plane_t;
	double d1, d2; //����·��
	double tempphase; // ��λ
	complex<double> tempejphase; // = exp(j*phase)
	RayTracing rayTracing(mirror);

	bool isInter = false;

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			Ex_R[i][j] = 0;
			Ey_R[i][j] = 0;
			Ez_R[i][j] = 0;
			R_Plane[i][j] = 0;
			rayTracing.calcNormalOfLine_Mirror(Plane[i][j],
				n_Plane[i][j], n_light,
				InterPoint, isInter, dir_t);
			if (!isInter)
				break;
			n_light.Normalization();

			// �������淨����ת����Դ����ϵ��(��ת������������ϵ)
			n_light_Plane = rotatMatrixSou * n_light;
			n_light_Plane.Normalization();  // ��λ��
			CalReflectExyz(n_light_Plane, Ex1[i][j], Ey1[i][j],
				Ez1[i][j], Ex_R[i][j], Ey_R[i][j], Ez_R[i][j]); // ֻ�������任

			Reflight = RayTracing::reflectLight(n_Plane[i][j], n_light);   // �������
			Rn_Plane[i][j] = Reflight;

			if (dir_t > 0.0000000001)  // ƽ���ڷ������ǰ��
			{
				plane_t = IntersectPlane(InterPoint, Reflight,
					Org_Source, R_Source, R_Plane[i][j]);
				d2 = CalDistance(InterPoint, R_Plane[i][j]);
				d1 = CalDistance(InterPoint, Plane[i][j]);

				if (plane_t > 0.0)  // ������2�ڷ������ǰ��
					tempphase = -(d1 + d2) / lamda * 2 * Pi;
				else
					tempphase = -(d1 - d2) / lamda * 2 * Pi;
				tempejphase = complex <double>(cos(tempphase), sin(tempphase));
				Ex_R[i][j] = ComMul(Ex_R[i][j], tempejphase);  // ֻ����λ�任
				Ey_R[i][j] = ComMul(Ey_R[i][j], tempejphase);
				Ez_R[i][j] = ComMul(Ez_R[i][j], tempejphase);
			}
			else if (dir_t < -0.0000000001)   // ƽ���ڷ�����ĺ���
			{
				plane_t = IntersectPlane(InterPoint, Reflight,
					Org_Source, R_Source, R_Plane[i][j]);
				d2 = CalDistance(InterPoint, R_Plane[i][j]);
				d1 = CalDistance(InterPoint, Plane[i][j]);

				if (plane_t < 0.0)  // ������2�ڷ�����ĺ���
					tempphase = (d1 + d2) / lamda * 2 * Pi;
				else
					tempphase = (d1 - d2) / lamda * 2 * Pi;
				tempejphase = complex <double>(cos(tempphase), sin(tempphase));
				Ex_R[i][j] = ComMul(Ex_R[i][j], tempejphase); // ֻ����λ�任
				Ey_R[i][j] = ComMul(Ey_R[i][j], tempejphase);
				Ez_R[i][j] = ComMul(Ez_R[i][j], tempejphase);
			}	
			else  // ƽ���뷴�����ཻ
			{
				R_Plane[i][j] = InterPoint;
			}
			
		}
	} // endloop

	CalAmplitude();  // ֻ�����ȱ任

	//Դ�Ĵ�������ı�
	n_Source = R_Source;
	updateSource_n(n_Source);
	
	ofstream outfile1("Ex_R.txt");
	ofstream outfile2("Ey_R.txt");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			outfile1 << Ex_R[i][j].real() << " " << Ex_R[i][j].imag() << "\n";
			outfile2 << Ey_R[i][j].real() << " " << Ey_R[i][j].imag() << "\n";
		}
	}
	outfile1.close();
	outfile2.close();/**/
}

complex<double> PVVA::ConjugateMul(const complex<double> &A, const complex<double> &B) const
{
	return complex<double>(A.real() * B.real() + A.imag() * B.imag(),
		-A.real() * B.imag() + A.imag() * B.real());
}

complex<double> PVVA::ComMul(const complex<double>& A, const complex<double>& B) const
{
	return complex<double>(A.real() * B.real() - A.imag() * B.imag(),
		A.real() * B.imag() + A.imag() * B.real());
}

double PVVA::CalDistance(const Vector3 &a, const Vector3 &b) const
{
	return pow(pow((a.x - b.x), 2) + pow((a.y - b.y), 2)+ pow((a.z - b.z), 2), 0.5);
}

bool PVVA::InterVal_PointinTriangle(const Vector2 & A, 
	const Vector2 & B, const Vector2 & C, const Vector2 & P)
{
	Vector2 v0 = C - A;
	Vector2 v1 = B - A;
	Vector2 v2 = P - A;

	double dot00 = v0.Dot(v0);
	double dot01 = v0.Dot(v1);
	double dot02 = v0.Dot(v2);
	double dot11 = v1.Dot(v1);
	double dot12 = v1.Dot(v2);

	double inverDeno = 1.0 / (dot00 * dot11 - dot01 * dot01);

	double u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (u < -0.0001 || u > 1.00001) // if u out of range, return directly
	{
		return false;
	}

	double v = (dot00 * dot12 - dot01 * dot02) * inverDeno;
	if (v < -0.0001 || v > 1.00001) // if v out of range, return directly
	{
		return false;
	}

	return u + v <= 1.00001;
}
