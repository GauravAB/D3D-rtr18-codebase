#include "Quad.h"

namespace quad
{
	HRESULT hr;
	ID3D11Buffer* pQuadBuffer;

	float vertices[] =
	{
		1.0,-1.0,0.0,
		1.0,1.0,0.0,
		-1.0,-1.0,0.0,
		-1.0,1.0,0.0
	};




	HRESULT init(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext)
	{
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(float) * 3 * 4;		//8 vertices of quad
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		hr = pID3D11Device->CreateBuffer(&bufferDesc, NULL, &pQuadBuffer);

		if (FAILED(hr))
		{
			return hr;
		}
		
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));


		pID3D11DeviceContext->Map(pQuadBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
		memcpy(mappedSubresource.pData,quad::vertices, sizeof(float) * 3 * 4);
		pID3D11DeviceContext->Unmap(pQuadBuffer, NULL);

	}

	void setVertexBufferAndTopology(ID3D11Device* pID3D11Device , ID3D11DeviceContext* pID3D11DeviceContext)
	{
		UINT stride = sizeof(float) * 3;
		UINT offset = 0;
		pID3D11DeviceContext->IASetVertexBuffers(0, 1, &pQuadBuffer, &stride, &offset);
		pID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	}

	void uninitialize(void)
	{
		if (pQuadBuffer)
		{
			pQuadBuffer->Release();
			pQuadBuffer = NULL;
		}
	}


}