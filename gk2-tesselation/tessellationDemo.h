#pragma once
#include "dxApplication.h"

namespace mini
{
	namespace gk2
	{
		class TessellationDemo : public DxApplication
		{
		public:
			explicit TessellationDemo(HINSTANCE appInstance);

		protected:
			void Update(const Clock& dt) override;
			void Render() override;

		private:
			void UpdateCameraCB();
			void UpdateTesselationCB();
			void ProcessKeyboardInput();

			dx_ptr<ID3D11Buffer> m_cbView, m_cbProj, m_cbSurfaceColor;
			dx_ptr<ID3D11Buffer> m_cbTesselationFactors;
			float m_tessOutside;
			float m_tessInside;

			dx_ptr<ID3D11Buffer> m_vertexBuffer;
			unsigned int m_vertexStride;
			unsigned int m_vertexCount;

			dx_ptr<ID3D11InputLayout> m_layout;
			dx_ptr<ID3D11RasterizerState> m_rsWireframe;

			dx_ptr<ID3D11VertexShader> m_tessVS;
			dx_ptr<ID3D11HullShader> m_tessHS;
			dx_ptr<ID3D11DomainShader> m_tessDS;
			dx_ptr<ID3D11PixelShader> m_tessPS;
		};
	}
}