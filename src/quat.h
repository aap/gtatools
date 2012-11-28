#ifndef QUATERNION_H
#define QUATERNION_H

#include <iostream>
#include <cmath>

template <class N>
struct quaternion
{
	N w, x, y, z;
	quaternion();
	quaternion(N, N, N);
	quaternion(N, N, N, N);
	quaternion(const quaternion<N>& quat);
	void print(std::ostream&) const;
	void conjugate();
	quaternion<N> getConjugate(void) const;
	N normsq() const;
	N norm() const;
	void normalize();
	void invert();
	N dot(const quaternion& rhs) const;
	N dot4(const quaternion& rhs) const;
	quaternion<N> wedge(const quaternion& rhs) const;
	quaternion<N> slerp(const quaternion& rhs, float f) const;
	quaternion& operator= (const quaternion& rhs);
	quaternion& operator+= (const N& rhs);
	quaternion& operator-= (const N& rhs);
	quaternion& operator*= (const N& rhs);
	quaternion& operator/= (const N& rhs);
	quaternion& operator+= (const quaternion& rhs);
	quaternion& operator-= (const quaternion& rhs);
	quaternion& operator*= (const quaternion& rhs);
	quaternion& operator/= (const quaternion& rhs);
	quaternion  operator+ (const quaternion& rhs) const;
	quaternion  operator+ (const N& rhs) const;
	quaternion  operator- (const quaternion& rhs) const;
	quaternion  operator- (const N& rhs) const;
	quaternion  operator- () const;
	quaternion  operator* (const quaternion& rhs) const;
	quaternion  operator* (const N& rhs) const;
	quaternion  operator/ (const quaternion& rhs) const;
	quaternion  operator/ (const N& rhs) const;
};

template <class N>
quaternion<N>::quaternion()
: w(0), x(0), y(0), z(0)
{
}

template <class N>
quaternion<N>::quaternion(N x, N y, N z)
: w(0), x(x), y(y), z(z)
{
}

template <class N>
quaternion<N>::quaternion(N w, N x, N y, N z)
: w(w), x(x), y(y), z(z)
{
}

template <class N>
quaternion<N>::quaternion(const quaternion<N>& quat)
{
	w = quat.w;
	x = quat.x;
	y = quat.y;
	z = quat.z;
}

template <class N>
void quaternion<N>::print(std::ostream& of) const
{
	of << "(" << w << ", " << x << ", " << y << ", " << z << ")";
}

template <class N>
void quaternion<N>::conjugate()
{
	x = -x;
	y = -y;
	z = -z;
}

template <class N>
quaternion<N> quaternion<N>::getConjugate(void) const
{
	return quaternion(w, -x, -y, -z);
}

template <class N>
N quaternion<N>::normsq() const
{
	N n;
	n = w*w + x*x + y*y + z*z; 
	return n;
}

template <class N>
N quaternion<N>::norm() const
{
	return sqrt(normsq());
}

template <class N>
void quaternion<N>::normalize()
{
	N n = norm();
	*this /= n;
}

template <class N>
void quaternion<N>::invert()
{
	conjugate();
	N n = normsq();
	*this /= n;
}

template <class N>
N quaternion<N>::dot(const quaternion& rhs) const
{
	return x*rhs.x + y*rhs.y + z*rhs.z;
}

template <class N>
N quaternion<N>::dot4(const quaternion& rhs) const
{
	return w*rhs.w + x*rhs.x + y*rhs.y + z*rhs.z;
}

template <class N>
quaternion<N> quaternion<N>::wedge(const quaternion& rhs) const
{
//	float xn = y*rhs.z - z*rhs.y;
//	float yn = z*rhs.x - x*rhs.z;
//	float zn = x*rhs.y - y*rhs.x;
	N xn = y*rhs.z - z*rhs.y;
	N yn = z*rhs.x - x*rhs.z;
	N zn = x*rhs.y - y*rhs.x;
	return quaternion(xn, yn, zn);
}

template <class N>
quaternion<N> quaternion<N>::slerp(const quaternion& rhs, float f) const
{
	// slerp
	quaternion<N> q1 = *this;
	float dot = q1.dot4(rhs);
	if (dot < 0) {
		dot = -dot;
		q1 = -q1;
	}
	float phi = acos(dot);

	if (phi > 0.00001)
		q1 = q1*sin((1.0f-f)*phi)/sin(phi)+rhs*sin(f*phi)/sin(phi);
	return q1;
}

template <class N>
quaternion<N>& quaternion<N>::operator= (const quaternion& rhs)
{
	w = rhs.w;
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator+= (const quaternion& rhs)
{
	w += rhs.w;
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator+= (const N& rhs)
{
	w += rhs;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator-= (const quaternion& rhs)
{
	w -= rhs.w;
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator-= (const N& rhs)
{
	w -= rhs;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator*= (const quaternion& rhs)
{
	N wn, xn, yn, zn;
	wn = w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z;
	xn = w*rhs.x + x*rhs.w + y*rhs.z - z*rhs.y;
	yn = w*rhs.y + y*rhs.w + z*rhs.x - x*rhs.z;
	zn = w*rhs.z + z*rhs.w + x*rhs.y - y*rhs.x;
	w = wn; x = xn; y = yn; z = zn;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator*= (const N& rhs)
{
	w *= rhs;
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator/= (const quaternion& rhs)
{
	quaternion<N> q = rhs;
	q.invert();
	*this *= q;
	return *this;
}

template <class N>
quaternion<N>& quaternion<N>::operator/= (const N& rhs)
{
	w /= rhs;
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}

template <class N>
std::ostream &operator<<(std::ostream& out, const quaternion<N>& q)
{
	q.print(out);
	return out;
}

template <class N>
quaternion<N> quaternion<N>::operator+(const quaternion<N>& rhs) const
{
	quaternion<N> newquat = *this;
	newquat += rhs;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator+(const N& rhs) const
{
	quaternion<N> newquat = *this;
	newquat += rhs;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator-(const quaternion<N>& rhs) const
{
	quaternion<N> newquat = *this;
	newquat -= rhs;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator-(const N& rhs) const
{
	quaternion<N> newquat = *this;
	newquat -= rhs;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator-() const
{
	quaternion<N> newquat = *this;
	newquat *= -1;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator*(const quaternion<N>& rhs) const
{
	quaternion<N> newquat = *this;
	newquat *= rhs;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator*(const N& rhs) const
{
	quaternion<N> newquat = *this;
	newquat *= rhs;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator/(const quaternion<N>& rhs) const
{
	quaternion<N> newquat = *this;
	newquat /= rhs;
	return newquat;
}

template <class N>
quaternion<N> quaternion<N>::operator/(const N& rhs) const
{
	quaternion<N> newquat = *this;
	newquat /= rhs;
	return newquat;
}
#endif
