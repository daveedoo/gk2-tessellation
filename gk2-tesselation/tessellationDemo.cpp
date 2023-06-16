#include "tessellationDemo.h"
#include <DirectXMath.h>

using namespace std;
using namespace mini;
using namespace gk2;
using namespace DirectX;

TessellationDemo::TessellationDemo(HINSTANCE appInstance)
	: DxApplication(appInstance, 1280, 720, L"TessellationDemo"),
	  m_cbView(m_device.CreateConstantBuffer<XMFLOAT4X4>()), m_cbProj(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	  m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
	  m_cbTesselationFactors(m_device.CreateConstantBuffer<XMFLOAT4>()),	// minimal CB size is 16B
	  m_tessOutside(2), m_tessInside(2),
	  m_vertexStride(sizeof(XMFLOAT3))
{
	InitVertexData();

	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMFLOAT4X4 tmpMtx;
	XMStoreFloat4x4(&tmpMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	UpdateBuffer(m_cbProj, tmpMtx);
	//UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	UpdateTesselationCB();
	UpdateCameraCB();

	// RASTERIZER
	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	m_rsWireframe = m_device.CreateRasterizerState(rsDesc);
	m_device.context()->RSSetState(m_rsWireframe.get());

	// SHADERS
	auto vsCode = m_device.LoadByteCode(L"tessellatedTriangleVS.cso");
	auto vsCodeNonTess = m_device.LoadByteCode(L"simpleVS.cso");
	m_tessVS = m_device.CreateVertexShader(vsCode);
	m_tessVSNonTess = m_device.CreateVertexShader(vsCodeNonTess);
	m_tessHS = m_device.CreateHullShader(m_device.LoadByteCode(L"tessellatedTriangleHS.cso"));
	m_tessDS = m_device.CreateDomainShader(m_device.LoadByteCode(L"tessellatedTriangleDS.cso"));
	m_tessPS = m_device.CreatePixelShader(m_device.LoadByteCode(L"tessellatedTrianglePS.cso"));


	m_device.context()->PSSetShader(m_tessPS.get(), nullptr, 0);

	// LAYOUT
	const D3D11_INPUT_ELEMENT_DESC layout[1] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_layout = m_device.CreateInputLayout(layout, vsCode);
	m_device.context()->IASetInputLayout(m_layout.get());

	// VERTEX + INDEX BUFFERS
	UpdateVertexBuffer();

	// CONSTANT BUFFERS
	ID3D11Buffer* tmpBuf = m_cbView.get();
	m_device.context()->VSSetConstantBuffers(0, 1, &tmpBuf);
	tmpBuf = m_cbTesselationFactors.get();
	m_device.context()->HSSetConstantBuffers(0, 1, &tmpBuf);
	tmpBuf = m_cbProj.get();
	m_device.context()->DSSetConstantBuffers(0, 1, &tmpBuf);
	m_device.context()->VSSetConstantBuffers(1, 1, &tmpBuf);
	tmpBuf = m_cbSurfaceColor.get();
	m_device.context()->PSSetConstantBuffers(0, 1, &tmpBuf);
}

void TessellationDemo::InitVertexData()
{
	// 1x1
	m_patch1 = vector<XMFLOAT3>{
		{ XMFLOAT3(-1.5f, -1.5f, 0.0f) },
		{ XMFLOAT3(1.5f, -1.5f, 0.0f) },
		{ XMFLOAT3(1.5f, 1.5f, 0.0f) },
		{ XMFLOAT3(-1.5f, 1.5f, 0.0f) },
	};
	vector<uint16_t> m_patch1Idx = {
		0, 1, 2, 3
	};
	m_vertexBuffer = m_device.CreateVertexBuffer<XMFLOAT3>(4);
	m_indexBuffer = m_device.CreateIndexBuffer(m_patch1Idx);
	UpdateBuffer<XMFLOAT3>(m_vertexBuffer, m_patch1);

	// 4x4
	//std::vector<DirectX::XMFLOAT3> m_patch16;
	float gap = 1.f;
	float x = -1.5 * gap;
	for (size_t i = 0; i < 4; i++)
	{
		float y = -1.5 * gap;
		m_patch16B.push_back(XMFLOAT3(x, y, 0.f));
		m_patch16A.push_back(XMFLOAT3(x, y, 0.f)); y += gap;
		m_patch16B.push_back(XMFLOAT3(x, y, -1.f));
		m_patch16A.push_back(XMFLOAT3(x, y, 1.f)); y += gap;
		m_patch16B.push_back(XMFLOAT3(x, y, 1.f));
		m_patch16A.push_back(XMFLOAT3(x, y, 1.f)); y += gap;
		m_patch16B.push_back(XMFLOAT3(x, y, 0.f));
		m_patch16A.push_back(XMFLOAT3(x, y, 0.f)); y += gap;
		//for (size_t j = 0; j < 4; j++)
		//{
		//	m_patch16.push_back(XMFLOAT3(x, y, 0.f));
		//	y += gap;
		//}
		x += gap;
	}
	vector<uint16_t> m_patch16Idx = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 10, 11,
		12, 13, 14, 15
	};
	vector<uint16_t> m_patch16ControlPointsIdx = {
		0, 4, 4, 5, 5, 1, 1, 0,	//8
		5, 6, 6, 2, 2, 1,		//6
		6, 7, 7, 3, 3, 2,		//6
		4, 8, 8, 9, 9, 5,		//6
		9, 10, 10, 6,			//4
		10, 11, 11, 7,			//4
		8, 12, 12, 13, 13, 9,	//6
		13, 14, 14, 10,			//4
		14, 15, 15, 11			//4
	};	// 48

	//vector<uint16_t> m_patch16Idx = {
	//	0, 4, 5, 1,
	//	1, 5, 6, 2,
	//	2, 6, 7, 3,
	//	4, 8, 9, 5,
	//	5, 9, 10, 6,
	//	6, 10, 11, 7,
	//	8, 12, 13, 9,
	//	9, 13, 14, 10,
	//	10, 14, 15, 11
	//};
	m_vertexBuffer16 = m_device.CreateVertexBuffer<XMFLOAT3>(16);
	m_indexBuffer16 = m_device.CreateIndexBuffer(m_patch16Idx);
	UpdateBuffer<XMFLOAT3>(m_vertexBuffer16, m_patch16A);

	m_indexBufferControlPoints = m_device.CreateIndexBuffer(m_patch16ControlPointsIdx);
}

void TessellationDemo::UpdateCameraCB()
{
	XMFLOAT4X4 viewMtx;
	XMStoreFloat4x4(&viewMtx, m_camera.getViewMatrix());
	UpdateBuffer(m_cbView, viewMtx);
}

void mini::gk2::TessellationDemo::UpdateTesselationCB()
{
	UpdateBuffer(m_cbTesselationFactors, XMFLOAT2(m_tessOutside, m_tessInside));
}

void TessellationDemo::ProcessKeyboardInput()
{
	static const float TESS_MIN = 1.f;
	static const float TESS_MAX = 32.f;

	static KeyboardState prev;
	KeyboardState kbState;
	m_keyboard.GetState(kbState);

	if (prev.keyPressed(kbState, DIK_DOWN))
	{
		m_tessOutside = clamp(--m_tessOutside, TESS_MIN, TESS_MAX);
		UpdateTesselationCB();
	}
	if (prev.keyPressed(kbState, DIK_UP))
	{
		m_tessOutside = clamp(++m_tessOutside, TESS_MIN, TESS_MAX);
		UpdateTesselationCB();
	}
	
	if (prev.keyPressed(kbState, DIK_LEFT))
	{
		m_tessInside = clamp(--m_tessInside, TESS_MIN, TESS_MAX);
		UpdateTesselationCB();
	}
	if (prev.keyPressed(kbState, DIK_RIGHT))
	{
		m_tessInside = clamp(++m_tessInside, TESS_MIN, TESS_MAX);
		UpdateTesselationCB();
	}

	if (prev.keyPressed(kbState, DIK_F))
	{
		static bool b = true;
		if (b)
			UpdateBuffer(m_vertexBuffer16, m_patch16A);
		else
			UpdateBuffer(m_vertexBuffer16, m_patch16B);

		b = !b;
	}

	prev = kbState;
}

void TessellationDemo::UpdateVertexBuffer()
{
	unsigned int offset = 0;

	//ID3D11Buffer* b = m_vertexBuffer.get();
	//m_device.context()->IASetVertexBuffers(0, 1, &b, &m_vertexStride, &offset);
	//b = m_indexBuffer.get();
	//m_device.context()->IASetIndexBuffer(b, DXGI_FORMAT_R16_UINT, 0);
	//m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	//m_vertexCount = 4;

	ID3D11Buffer* b = m_vertexBuffer16.get();
	m_device.context()->IASetVertexBuffers(0, 1, &b, &m_vertexStride, &offset);
	b = m_indexBuffer16.get();
	m_device.context()->IASetIndexBuffer(b, DXGI_FORMAT_R16_UINT, 0);
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
	m_vertexCount = 16;
}

void TessellationDemo::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	if (HandleCameraInput(dt))
		UpdateCameraCB();

	ProcessKeyboardInput();
}

void TessellationDemo::Render()
{
	DxApplication::Render();
	unsigned int offset = 0;
	ID3D11Buffer* b;


	// control points draw
	m_device.context()->VSSetShader(m_tessVSNonTess.get(), nullptr, 0);
	m_device.context()->HSSetShader(nullptr, nullptr, 0);
	m_device.context()->DSSetShader(nullptr, nullptr, 0);

	b = m_indexBufferControlPoints.get();
	m_device.context()->IASetIndexBuffer(b, DXGI_FORMAT_R16_UINT, 0);
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f));
	m_vertexCount = 48;
	m_device.context()->DrawIndexed(m_vertexCount, 0, 0);


	// tessalation patch
	m_device.context()->VSSetShader(m_tessVS.get(), nullptr, 0);
	m_device.context()->HSSetShader(m_tessHS.get(), nullptr, 0);
	m_device.context()->DSSetShader(m_tessDS.get(), nullptr, 0);

	b = m_indexBuffer16.get();
	m_device.context()->IASetIndexBuffer(b, DXGI_FORMAT_R16_UINT, 0);
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	m_vertexCount = 16;
	m_device.context()->DrawIndexed(m_vertexCount, 0, 0);

}