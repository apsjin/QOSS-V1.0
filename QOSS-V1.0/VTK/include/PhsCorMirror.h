/*
*	created by liyun 2018/3/17
*   function ��λ������
*   version 1.0
*/

#ifndef PhsCorMirror_H  
#define PhsCorMirror_H

#include "Mirror.h"
#include <vector>
#include <string>

class PhsCorMirror : public Mirror
{
public:
	PhsCorMirror(); 

	virtual ~PhsCorMirror();

	virtual void calPolyData(double ds = 0);

	virtual void updateData();

	virtual QTreeWidgetItem* getTree();

	void sampling(double ds, double length, const Vector3& central, Mirror* mirror);

	bool sampling(double ds, double length, const Vector3& central,
		const Vector3& direction, const GraphTrans& graphTrans, Mirror* mirror);

	const vector<vector<Vector3>> & getLattice() const { return lattice; }

	void setLattice(const vector<vector<Vector3>> &);

private:

	vector<vector<Vector3>> lattice;


};



#endif // PhsCorMirror_H
