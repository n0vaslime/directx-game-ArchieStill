#include "pch.h"
#include "CMOGO.h"
#include "model.h"
#include <windows.h>
#include "Effects.h"
#include "camera.h"
#include "CommonStates.h"
#include "DrawData.h"
#include "Helper.h"

#include <iostream>

#include <DirectXCollision.h>

using namespace DirectX;

ID3D11RasterizerState*  CMOGO::s_pRasterState = nullptr;
int CMOGO::m_count = 0;

using namespace DirectX;
CMOGO::CMOGO(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) :m_model(nullptr)
{
	//if we've not created it yet do so now
	if (!s_pRasterState)
	{
		//Setup Raster State
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		// Create the rasterizer state from the description we just filled out.
		HRESULT hr = _pd3dDevice->CreateRasterizerState(&rasterDesc, &s_pRasterState);
		assert(hr == S_OK);
	}

	string filePath = "../Assets/" + _fileName + ".cmo";

	wchar_t* file = Helper::charToWChar(filePath.c_str());

	m_model = Model::CreateFromCMO(_pd3dDevice, file, *_EF);

	//Construct Model Bounding Box
	std::vector<XMFLOAT3> bbVerts(8 * m_model->meshes.size());

	for (int i = 0; i < m_model->meshes.size(); i++)
	{
		const auto& bbox = m_model->meshes[i]->boundingBox;

		const auto& bbox_ctr = bbox.Center;
		const auto& bbox_ext = bbox.Extents;

		bbVerts[8 * i + 0] = bbox.Center + XMFLOAT3(-1, -1, -1) * bbox_ext;
		bbVerts[8 * i + 1] = bbox.Center + XMFLOAT3(+1, -1, -1) * bbox_ext;
		bbVerts[8 * i + 2] = bbox.Center + XMFLOAT3(-1, +1, -1) * bbox_ext;
		bbVerts[8 * i + 3] = bbox.Center + XMFLOAT3(+1, +1, -1) * bbox_ext;
		bbVerts[8 * i + 4] = bbox.Center + XMFLOAT3(-1, -1, +1) * bbox_ext;
		bbVerts[8 * i + 5] = bbox.Center + XMFLOAT3(+1, -1, +1) * bbox_ext;
		bbVerts[8 * i + 6] = bbox.Center + XMFLOAT3(-1, +1, +1) * bbox_ext;
		bbVerts[8 * i + 7] = bbox.Center + XMFLOAT3(+1, +1, +1) * bbox_ext;
	}

	// Set up minmax floats
	auto minmax_x = std::minmax_element(bbVerts.begin(), bbVerts.end(), [](const XMFLOAT3& a, const XMFLOAT3& b) { return a.x < b.x; });
	auto minmax_y = std::minmax_element(bbVerts.begin(), bbVerts.end(), [](const XMFLOAT3& a, const XMFLOAT3& b) { return a.y < b.y; });
	auto minmax_z = std::minmax_element(bbVerts.begin(), bbVerts.end(), [](const XMFLOAT3& a, const XMFLOAT3& b) { return a.z < b.z; });

	// Get min and max values
	float minX = minmax_x.first->x;
	float maxX = minmax_x.second->x;
	float minY = minmax_y.first->y;
	float maxY = minmax_y.second->y;
	float minZ = minmax_z.first->z;
	float maxZ = minmax_z.second->z;

	// Get center values
	float centerX = (maxX + minX) / 2;
	float centerY = (maxY + minY) / 2;
	float centerZ = (maxZ + minZ) / 2;

	// Get Extents
	float extX = (maxX - minX) / 2;
	float extY = (maxY - minY) / 2;
	float extZ = (maxZ - minZ) / 2;

	m_collider.Center = { centerX, centerY, centerZ };
	m_collider.Extents = { extX, extY, extZ };
}

CMOGO::~CMOGO()
{
	//model shouldn't need deleting as it's linked to by a unique_ptr
	m_count--;

	//okay I've just deleted the last CMOGO let's get rid of this
	if (m_count == 0 && s_pRasterState)
	{
		s_pRasterState->Release();
		s_pRasterState = nullptr;
	}
}

void CMOGO::Tick(GameData* _GD)
{
	GameObject::Tick(_GD);
}

void CMOGO::Draw(DrawData* _DD)
{
	//a dirty hack as the CMO model drawer breaks the depth stencil state
	ID3D11DepthStencilState *DSS = nullptr;
	UINT ref;

	//pick up a copy of the current state...
	_DD->m_pd3dImmediateContext->OMGetDepthStencilState(&DSS, &ref);

	m_model->Draw(_DD->m_pd3dImmediateContext, *_DD->m_states, //graphics device and CommonStates reuqired by model system
		m_worldMat, //world transform to poisiton this model in the world
		_DD->m_cam->GetView(), _DD->m_cam->GetProj(), //veiw and projection matrix of the camera
		false, //NO! I don't want wireframe
		[&](){_DD->m_pd3dImmediateContext->RSSetState(s_pRasterState);} //this VERY weird construction creates a function on the fly to set up the render states correctly else the model system overrides them BADLY
		);

	//...and put the depth stencil state back again
	_DD->m_pd3dImmediateContext->OMSetDepthStencilState(DSS, ref);

	//clear this copy away
	if (DSS) 
	{
		DSS->Release();
	}
}

bool CMOGO::Intersects(const CMOGO& other) const
{
	//for (const auto& this_mesh : m_model->meshes) 
	//for (const auto& other_mesh : other.m_model->meshes)
	//{
	//	BoundingBox thisBox = this_mesh->boundingBox;
	//	BoundingBox otherBox = other_mesh->boundingBox;

	//	thisBox.Extents = thisBox.Extents * m_scale;
	//	otherBox.Extents = otherBox.Extents * other.m_scale;

	//	thisBox.Center = thisBox.Center * m_scale + m_pos;
	//	otherBox.Center = otherBox.Center * other.m_scale + other.m_pos;

	//	//string thisCtr = std::to_string(thisBox.Center.x) + ", " + std::to_string(thisBox.Center.y) + ", " + std::to_string(thisBox.Center.z);
	//	//string otherCtr = std::to_string(otherBox.Center.x) + ", " + std::to_string(otherBox.Center.y) + ", " + std::to_string(otherBox.Center.z);

	//	//string thisExt = std::to_string(thisBox.Extents.x) + ", " + std::to_string(thisBox.Extents.y) + ", " + std::to_string(thisBox.Extents.z);
	//	//string otherExt = std::to_string(otherBox.Extents.x) + ", " + std::to_string(otherBox.Extents.y) + ", " + std::to_string(otherBox.Extents.z);
	//	//
	//	//std::cout << thisCtr << ", " << thisExt << std::endl;
	//	//std::cout << otherCtr << " " << otherExt << std::endl;

	//	if (thisBox.Intersects(otherBox)) return true;
	//}

	//return false;

	BoundingOrientedBox c1, c2;

	m_collider.Transform(c1, m_worldMat);
	other.m_collider.Transform(c2, other.m_worldMat);

	return (c1.Intersects(c2));
}
