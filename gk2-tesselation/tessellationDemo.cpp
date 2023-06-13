#include "tessellationDemo.h"

using namespace std;
using namespace mini;
using namespace gk2;
using namespace DirectX;

TessellationDemo::TessellationDemo(HINSTANCE appInstance)
	: DxApplication(appInstance, 1280, 720, L"TessellationDemo"),
	  m_cbView(m_device.CreateConstantBuffer<XMFLOAT4X4>()), m_cbProj(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	  m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()), m_vertexStride(sizeof(XMFLOAT3)), m_vertexCount(4)
{
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMFLOAT4X4 tmpMtx;
	XMStoreFloat4x4(&tmpMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	UpdateBuffer(m_cbProj, tmpMtx);
	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));

	UpdateCameraCB();

	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	m_rsWireframe = m_device.CreateRasterizerState(rsDesc);

	auto vsCode = m_device.LoadByteCode(L"tessellatedTriangleVS.cso");
	m_tessVS = m_device.CreateVertexShader(vsCode);
	m_tessHS = m_device.CreateHullShader(m_device.LoadByteCode(L"tessellatedTriangleHS.cso"));
	m_tessDS = m_device.CreateDomainShader(m_device.LoadByteCode(L"tessellatedTriangleDS.cso"));
	m_tessPS = m_device.CreatePixelShader(m_device.LoadByteCode(L"tessellatedTrianglePS.cso"));

	m_device.context()->VSSetShader(m_tessVS.get(), nullptr, 0);
	m_device.context()->HSSetShader(m_tessHS.get(), nullptr, 0);
	m_device.context()->DSSetShader(m_tessDS.get(), nullptr, 0);
	m_device.context()->PSSetShader(m_tessPS.get(), nullptr, 0);

	const D3D11_INPUT_ELEMENT_DESC layout[1] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_layout = m_device.CreateInputLayout(layout, vsCode);

	vector<XMFLOAT3> vtx{
		{ XMFLOAT3(-1.5f, -1.5f, 0.0f) },
		{ XMFLOAT3(1.5f, -1.5f, 0.0f) },
		{ XMFLOAT3(1.5f, 1.5f, 0.0f) },
		{ XMFLOAT3(-1.5f, 1.5f, 0.0f) },
	};

	m_vertexBuffer = m_device.CreateVertexBuffer(vtx);
	m_device.context()->IASetInputLayout(m_layout.get());

	ID3D11Buffer* tmpBuf = m_cbView.get();
	m_device.context()->VSSetConstantBuffers(0, 1, &tmpBuf);
	tmpBuf = m_cbProj.get();
	m_device.context()->DSSetConstantBuffers(0, 1, &tmpBuf);
	tmpBuf = m_cbSurfaceColor.get();
	m_device.context()->PSSetConstantBuffers(0, 1, &tmpBuf);

	m_device.context()->RSSetState(m_rsWireframe.get());
	unsigned int offset = 0;
	ID3D11Buffer* b = m_vertexBuffer.get();
	m_device.context()->IASetVertexBuffers(0, 1, &b, &m_vertexStride, &offset);
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
}

void TessellationDemo::UpdateCameraCB()
{
	XMFLOAT4X4 viewMtx;
	XMStoreFloat4x4(&viewMtx, m_camera.getViewMatrix());
	UpdateBuffer(m_cbView, viewMtx);
}

void TessellationDemo::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	if (HandleCameraInput(dt))
		UpdateCameraCB();
}

void TessellationDemo::Render()
{
	DxApplication::Render();

	m_device.context()->Draw(m_vertexCount, 0);
}