#pragma once
#include "common.h"


namespace quad
{
	HRESULT init(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext);
	void setVertexBufferAndTopology(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext);
	void uninitialize(void);
}