class Camera
{
	protected:
		D3DXVECTOR3 position;

		D3DXVECTOR3 look_at;
		D3DXVECTOR3 right;
		D3DXVECTOR3 up;

		float look_at_rotation;
		float right_rotation;
		float up_rotation;

		D3DXMATRIX view_matrix;

		LPDIRECT3DDEVICE9 rendering_device;

		bool update_required;

		HRESULT update_view_matrix();


	public:
		Camera(LPDIRECT3DDEVICE9 rendering_device);

		void set_position(
			FLOAT x,
			FLOAT y,
			FLOAT z
		);
		void look_at_position(
			D3DXVECTOR3 *position,
			D3DXVECTOR3 *look_at,
			D3DXVECTOR3 *up
		);

		HRESULT update();
};


HRESULT Camera::update_view_matrix()
{
	D3DXMATRIX total_matrix, look_at_matrix, right_matrix, up_matrix;
	float new_view_element_1, new_view_element_2, new_view_element_3;

	D3DXMatrixRotationAxis(
		&look_at_matrix,
		&look_at,
		look_at_rotation
	);
	D3DXMatrixRotationAxis(
		&right_matrix,
		&right,
		right_rotation
	);
	D3DXMatrixRotationAxis(
		&up_matrix,
		&up,
		up_rotation
	);

	D3DXMatrixMultiply(
		&total_matrix,
		&look_at_matrix,
		&right_matrix
	);
	D3DXMatrixMultiply(
		&total_matrix,
		&total_matrix,
		&up_matrix
	);

	D3DXVec3TransformCoord(
		&right,
		&right,
		&total_matrix
	);
	D3DXVec3TransformCoord(
		&up,
		&up,
		&total_matrix
	);
	D3DXVec3Cross(
		&look_at,
		&right,
		&up
	);

	
	if (fabs(D3DXVec3Dot(&up, &right)) > 0.01)
		D3DXVec3Cross(
			&up,
			&look_at,
			&right
		);

	D3DXVec3Normalize(
		&look_at,
		&look_at
	);
	D3DXVec3Normalize(
		&right,
		&right
	);
	D3DXVec3Normalize(
		&up,
		&up
	);

	new_view_element_1 = -D3DXVec3Dot(
		&right,
		&position
	);
	new_view_element_2 = -D3DXVec3Dot(
		&up,
		&position
	);
	new_view_element_3 = -D3DXVec3Dot(
		&look_at,
		&position
	);
	view_matrix = D3DXMATRIX(
		right.x, up.x, look_at.x, 0.0f,
		right.y, up.y, look_at.y, 0.0f,
		right.z, up.z, look_at.z, 0.0f,
		new_view_element_1, new_view_element_2, new_view_element_3, 1.0f
	);

	up_rotation = right_rotation = look_at_rotation = 0.0f;

	update_required = false;

	return rendering_device->SetTransform(
		D3DTS_VIEW,
		&view_matrix
	);
}


Camera::Camera(LPDIRECT3DDEVICE9 device)
{
	position = D3DXVECTOR3(
		0.0f,
		0.0f,
		0.0f
	);

	look_at = D3DXVECTOR3(
		0.0f,
		0.0f,
		1.0f
	);
	right = D3DXVECTOR3(
		1.0f,
		0.0f,
		0.0f
	);
	up = D3DXVECTOR3(
		0.0f,
		1.0f,
		0.0f
	);

	update_required = false;

	up_rotation = right_rotation = look_at_rotation = 0.0f;

	D3DXMatrixIdentity(&view_matrix);

	rendering_device = device;
}


void Camera::set_position(FLOAT x, FLOAT y, FLOAT z)
{
	position = D3DXVECTOR3(
		x,
		y,
		z
	);
	update_required = true;
}


void Camera::look_at_position(D3DXVECTOR3 *new_position, D3DXVECTOR3 *new_look_at, D3DXVECTOR3 *new_up)
{
	D3DXMatrixLookAtLH(
		&view_matrix,
		new_position,
		new_look_at,
		new_up
	);

	position = *(new_position);

	look_at.x = view_matrix._13;
	look_at.y = view_matrix._23;
	look_at.z = view_matrix._33;

	right.x = view_matrix._11;
	right.y = view_matrix._21;
	right.z = view_matrix._31;

	up.x = view_matrix._12;
	up.y = view_matrix._22;
	up.z = view_matrix._32;

	up_rotation = right_rotation = look_at_rotation = 0.0f;
}


HRESULT Camera::update()
{
	if(rendering_device)
	{
		if(update_required)
			return update_view_matrix();

		return rendering_device->SetTransform(
			D3DTS_VIEW,
			&view_matrix
		);
	}

	return E_FAIL;
}