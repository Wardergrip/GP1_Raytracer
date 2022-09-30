#include "Matrix.h"

#include <cassert>

#include "MathHelpers.h"
#include <cmath>

namespace dae {
	Matrix::Matrix(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, const Vector3& t) :
		Matrix({ xAxis, 0 }, { yAxis, 0 }, { zAxis, 0 }, { t, 1 })
	{
	}

	Matrix::Matrix(const Vector4& xAxis, const Vector4& yAxis, const Vector4& zAxis, const Vector4& t)
	{
		data[0] = xAxis;
		data[1] = yAxis;
		data[2] = zAxis;
		data[3] = t;
	}

	Matrix::Matrix(const Matrix& m)
	{
		data[0] = m[0];
		data[1] = m[1];
		data[2] = m[2];
		data[3] = m[3];
	}

	Vector3 Matrix::TransformVector(const Vector3& v) const
	{
		return TransformVector(v[0], v[1], v[2]);
	}

	Vector3 Matrix::TransformVector(float x, float y, float z) const
	{
		return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z,
			data[0].y * x + data[1].y * y + data[2].y * z,
			data[0].z * x + data[1].z * y + data[2].z * z
		};
	}

	Vector3 Matrix::TransformPoint(const Vector3& p) const
	{
		return TransformPoint(p[0], p[1], p[2]);
	}

	Vector3 Matrix::TransformPoint(float x, float y, float z) const
	{
		return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
			data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
			data[0].z * x + data[1].z * y + data[2].z * z + data[3].z,
		};
	}

	const Matrix& Matrix::Transpose()
	{
		Matrix result{};
		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				result[r][c] = data[c][r];
			}
		}

		data[0] = result[0];
		data[1] = result[1];
		data[2] = result[2];
		data[3] = result[3];

		return *this;
	}

	Matrix Matrix::Transpose(const Matrix& m)
	{
		Matrix out{ m };
		out.Transpose();

		return out;
	}

	Vector3 Matrix::GetAxisX() const
	{
		return data[0];
	}

	Vector3 Matrix::GetAxisY() const
	{
		return data[1];
	}

	Vector3 Matrix::GetAxisZ() const
	{
		return data[2];
	}

	Vector3 Matrix::GetTranslation() const
	{
		return data[3];
	}

	Matrix Matrix::CreateTranslation(float x, float y, float z)
	{
		Matrix matrix{};

		matrix[3][0] = x;
		matrix[3][1] = y;
		matrix[3][2] = z;

		return matrix;
	}

	Matrix Matrix::CreateTranslation(const Vector3& t)
	{
		return { Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, t };
	}

	Matrix Matrix::CreateRotationX(float pitch)
	{
		Matrix matrix{};

		matrix[1][1] = cosf(pitch);
		matrix[1][2] = -sinf(pitch);
		matrix[2][1] = sinf(pitch);
		matrix[2][2] = cosf(pitch);

		return matrix;
	}

	Matrix Matrix::CreateRotationY(float yaw)
	{
		Matrix matrix{};

		matrix[0][0] = cosf(yaw);
		matrix[0][2] = -sinf(yaw);
		matrix[2][0] = sinf(yaw);
		matrix[2][2] = cosf(yaw);

		return matrix;
	}

	Matrix Matrix::CreateRotationZ(float roll)
	{
		Matrix matrix{};

		matrix[0][0] = cosf(roll);
		matrix[0][1] = sinf(roll);
		matrix[1][0] = -sinf(roll);
		matrix[1][1] = cosf(roll);

		return matrix;
	}

	Matrix Matrix::CreateRotation(const Vector3& r)
	{
		Matrix matrix{};

		matrix = CreateRotationX(r.x) * CreateRotationY(r.y) * CreateRotationZ(r.z);

		return matrix;
	}

	Matrix Matrix::CreateRotation(float pitch, float yaw, float roll)
	{
		return CreateRotation({ pitch, yaw, roll });
	}

	Matrix Matrix::CreateScale(float sx, float sy, float sz)
	{
		Matrix matrix{};

		matrix[0][0] = sx;
		matrix[1][1] = sy;
		matrix[2][2] = sz;

		return matrix;
	}

	Matrix Matrix::CreateScale(const Vector3& s)
	{
		return CreateScale(s[0], s[1], s[2]);
	}

#pragma region Operator Overloads
	Vector4& Matrix::operator[](int index)
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Vector4 Matrix::operator[](int index) const
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Matrix Matrix::operator*(const Matrix& m) const
	{
		Matrix result{};
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				result[r][c] = Vector4::Dot(data[r], m_transposed[c]);
			}
		}

		return result;
	}

	const Matrix& Matrix::operator*=(const Matrix& m)
	{
		Matrix copy{ *this };
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				data[r][c] = Vector4::Dot(copy[r], m_transposed[c]);
			}
		}

		return *this;
	}
#pragma endregion
}