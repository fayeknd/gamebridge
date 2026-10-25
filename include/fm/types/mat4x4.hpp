#pragma once
#include "../vec.hpp"
#include "fm/fmath.hpp"

namespace fm {

    template <typename T>
    class _mat4x4decomp {
    public:
        _vec3<T> position;
        _vec3<T> rotation;
        _vec3<T> scale;
        _vec3<T> skew;
    };
    
    // very proud of the matrix class. 
    // TODO : go through and comment all of this.
    // most of this is from wikipedia and stackoverflow
    // and some really niche blog sites i could not
    // possibly rediscover, so this is in desperate need
    // of documenting.

    static inline bool flipY = false;
    static inline bool flipX = false;
    static inline bool flipZ = false;

    template <typename T>
    class _mat4x4 {
    public:

        T m_data[4][4];

        _mat4x4();
        _mat4x4(T i);
        _mat4x4(T i[4][4]);
        _mat4x4(
            T i00, T i01, T i02, T i03,
            T i04, T i05, T i06, T i07,
            T i08, T i09, T i10, T i11,
            T i12, T i13, T i14, T i15
        );
        inline static _mat4x4<T> identity() { return _mat4x4<T>(1); };
        const T* data() const;

        _mat4x4<T> translated(_vec3<T> vec);
        _mat4x4<T>& translate(_vec3<T> vec);
        
        _mat4x4<T> scaled(_vec3<T> scale);
        _mat4x4<T>& scale(_vec3<T> scale);

        _mat4x4<T> rotatedAroundX(T degrees);
        _mat4x4<T>& rotateAroundX(T degrees);

        _mat4x4<T> rotatedAroundY(T degrees);
        _mat4x4<T>& rotateAroundY(T degrees);

        _mat4x4<T> rotatedAroundZ(T degrees);
        _mat4x4<T>& rotateAroundZ(T degrees);

        _mat4x4<T> rotated(_vec3<T> euler);
        _mat4x4<T>& rotate(_vec3<T> euler);

        _mat4x4<T> skewed(_vec3<T> v);
        _mat4x4<T>& skew(_vec3<T> v);

        _mat4x4<T> transposed();
        _mat4x4<T>& transpose();

        _mat4x4<T> inversed();
        _mat4x4<T>& inverse();

        _mat4x4decomp<T> decompose();
        _vec3<T> extractEuler();

        T& at(int row, int column);
        _mat4x4<T>& operator=(const _mat4x4<T>& m);
        _mat4x4<T>& operator*=(const _mat4x4<T>& m);
    };

    template <typename T>
    inline _mat4x4<T>::_mat4x4() {
        m_data[0][0] = 1; m_data[0][1] = 0; m_data[0][2] = 0; m_data[0][3] = 0;
        m_data[1][0] = 0; m_data[1][1] = 1; m_data[1][2] = 0; m_data[1][3] = 0;
        m_data[2][0] = 0; m_data[2][1] = 0; m_data[2][2] = 1; m_data[2][3] = 0;
        m_data[3][0] = 0; m_data[3][1] = 0; m_data[3][2] = 0; m_data[3][3] = 1;
    } 
    template <typename T>
    inline _mat4x4<T>::_mat4x4(T i) {
        m_data[0][0] = i; m_data[0][1] = 0; m_data[0][2] = 0; m_data[0][3] = 0;
        m_data[1][0] = 0; m_data[1][1] = i; m_data[1][2] = 0; m_data[1][3] = 0;
        m_data[2][0] = 0; m_data[2][1] = 0; m_data[2][2] = i; m_data[2][3] = 0;
        m_data[3][0] = 0; m_data[3][1] = 0; m_data[3][2] = 0; m_data[3][3] = i;
    }
    template <typename T>
    inline _mat4x4<T>::_mat4x4(T i[4][4]) {
        m_data[0][0] = i[0][0]; m_data[0][1] = i[0][1]; m_data[0][2] = i[0][2]; m_data[0][3] = i[0][3];
        m_data[1][0] = i[1][0]; m_data[1][1] = i[1][1]; m_data[1][2] = i[1][2]; m_data[1][3] = i[1][3];
        m_data[2][0] = i[2][0]; m_data[2][1] = i[2][1]; m_data[2][2] = i[2][2]; m_data[2][3] = i[2][3];
        m_data[3][0] = i[3][0]; m_data[3][1] = i[3][1]; m_data[3][2] = i[3][2]; m_data[3][3] = i[3][3];
    }
    template <typename T>
    inline _mat4x4<T>::_mat4x4(
            T i00, T i01, T i02, T i03,
            T i04, T i05, T i06, T i07,
            T i08, T i09, T i10, T i11,
            T i12, T i13, T i14, T i15) {
        m_data[0][0] = i00; m_data[0][1] = i01; m_data[0][2] = i02; m_data[0][3] = i03;
        m_data[1][0] = i04; m_data[1][1] = i05; m_data[1][2] = i06; m_data[1][3] = i07;
        m_data[2][0] = i08; m_data[2][1] = i09; m_data[2][2] = i10; m_data[2][3] = i11;
        m_data[3][0] = i12; m_data[3][1] = i13; m_data[3][2] = i14; m_data[3][3] = i15;
    }
    template <typename T>
    inline const T* _mat4x4<T>::data() const {
        return &m_data[0][0];
    } 

    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::translated(_vec3<T> vec) {
        _mat4x4<T> _ = *this;
        _.m_data[3][0] += vec.x;
        _.m_data[3][1] += vec.y;
        _.m_data[3][2] += vec.z;
        return _;
    }

    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::translate(_vec3<T> vec) {
        *this = translated(vec);
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::scaled(_vec3<T> scale) {
        _mat4x4<T> _ = *this;

        _.m_data[0][0] *= scale.x; _.m_data[0][1] *= scale.x; _.m_data[0][2] *= scale.x; _.m_data[0][3] *= scale.x;
        _.m_data[1][0] *= scale.y; _.m_data[1][1] *= scale.y; _.m_data[1][2] *= scale.y; _.m_data[1][3] *= scale.y;
        _.m_data[2][0] *= scale.z; _.m_data[2][1] *= scale.z; _.m_data[2][2] *= scale.z; _.m_data[2][3] *= scale.z;

        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::scale(_vec3<T> scale) {
        *this = scaled(scale);
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::rotatedAroundX(T degrees) {
        _mat4x4<T> _ = *this;
        T rad = degreesToRadians(degrees);
        T c = std::cos(rad);
        T s = std::sin(rad);

        for (int i = 0; i < 4; ++i) {
            T y = _.m_data[1][i];
            T z = _.m_data[2][i];
            _.m_data[1][i] = y * c - z * s;
            _.m_data[2][i] = y * s + z * c;
        }
        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::rotateAroundX(T degrees) {
        *this = this->rotatedAroundX(degrees);
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::rotatedAroundY(T degrees) {
        _mat4x4<T> _ = *this;
        T rad = degreesToRadians(degrees);
        T c = std::cos(rad);
        T s = std::sin(rad);

        for (int i = 0; i < 4; ++i) {
            T x = _.m_data[0][i];
            T z = _.m_data[2][i];
            _.m_data[0][i] = x * c - z * s;
            _.m_data[2][i] = x * s + z * c;
        }
        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::rotateAroundY(T degrees) {
        *this = this->rotatedAroundY(degrees);
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::rotatedAroundZ(T degrees) {
        _mat4x4<T> _ = *this;
        T rad = degreesToRadians(degrees);
        T c = std::cos(rad);
        T s = std::sin(rad);

        for (int i = 0; i < 4; ++i) {
            T x = _.m_data[0][i];
            T y = _.m_data[1][i];
            _.m_data[0][i] = x * c + y * s;
            _.m_data[1][i] = -x * s + y * c;
        }
        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::rotateAroundZ(T degrees) {
        *this = this->rotatedAroundZ(degrees);
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::rotated(_vec3<T> euler) {
        _mat4x4<T> _ = *this;
        _.rotateAroundY(euler.y);
        _.rotateAroundX(euler.x);
        _.rotateAroundZ(euler.z);
        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::rotate(_vec3<T> euler) {
        *this = this->rotated(euler);
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::skewed(_vec3<T> v)  {
        _mat4x4<T> _ = *this;

        T skewX = std::tan(degreesToRadians(v.x));
        T skewY = std::tan(degreesToRadians(v.y));
        T skewZ = std::tan(degreesToRadians(v.z));

        for (int i = 0; i < 4; ++i) {
            T x = _.m_data[0][i];
            T y = _.m_data[1][i];
            T z = _.m_data[2][i];

            _.m_data[0][i] += y * skewX;
            _.m_data[1][i] += z * skewY;
            _.m_data[2][i] += x * skewZ;
        }

        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::skew(_vec3<T> v) {
        *this = this->skewed(v);
        return *this;
    }
    template <typename T>
    inline T& _mat4x4<T>::at(int row, int column) {
        return m_data[row][column];
    }

    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::operator=(const _mat4x4<T>& m) {
        m_data[0][0] = m.m_data[0][0]; m_data[0][1] = m.m_data[0][1]; m_data[0][2] = m.m_data[0][2]; m_data[0][3] = m.m_data[0][3];
        m_data[1][0] = m.m_data[1][0]; m_data[1][1] = m.m_data[1][1]; m_data[1][2] = m.m_data[1][2]; m_data[1][3] = m.m_data[1][3];
        m_data[2][0] = m.m_data[2][0]; m_data[2][1] = m.m_data[2][1]; m_data[2][2] = m.m_data[2][2]; m_data[2][3] = m.m_data[2][3];
        m_data[3][0] = m.m_data[3][0]; m_data[3][1] = m.m_data[3][1]; m_data[3][2] = m.m_data[3][2]; m_data[3][3] = m.m_data[3][3];
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> operator*(const _mat4x4<T>& m1, const _mat4x4<T>& m2) {
        _mat4x4<T> _;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                _.m_data[col][row] = 
                    m1.m_data[0][row] * m2.m_data[col][0] +
                    m1.m_data[1][row] * m2.m_data[col][1] +
                    m1.m_data[2][row] * m2.m_data[col][2] +
                    m1.m_data[3][row] * m2.m_data[col][3];
            }
        }
        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::operator*=(const _mat4x4<T>& m) {
        *this = *this * m;
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> operator*(_mat4x4<T> m, T t) { // Removed 'const' and '&'
        m.m_data[0][0] *= t; m.m_data[1][0] *= t; m.m_data[2][0] *= t; m.m_data[3][0] *= t;
        m.m_data[0][1] *= t; m.m_data[1][1] *= t; m.m_data[2][1] *= t; m.m_data[3][1] *= t;
        m.m_data[0][2] *= t; m.m_data[1][2] *= t; m.m_data[2][2] *= t; m.m_data[3][2] *= t;
        m.m_data[0][3] *= t; m.m_data[1][3] *= t; m.m_data[2][3] *= t; m.m_data[3][3] *= t;
        return m;
    } 
    template <typename T>
    inline _mat4x4<T> model(_vec3<T> pos, _vec3<T> scale, _vec3<T> rotation, _vec3<T> skew = {0}) {
        return _mat4x4<T>::identity().scale(scale).skew(skew).rotate(rotation).translate(pos);
    }
    template <typename T>
    inline _mat4x4<T> lookAt(_vec3<T> pos, _vec3<T> target, _vec3<T> up = {0.0f, 1.0f, 0.0f}) {
        _mat4x4<T> _ = _mat4x4<T>::identity(); 

        _vec3<T> f = (pos - target).normalized(); 
        _vec3<T> r = up.cross(f).normalized(); 
        _vec3<T> u = f.cross(r); 

        T translateX = -r.dot(pos);
        T translateY = -u.dot(pos);
        T translateZ = -f.dot(pos);

        if (flipZ) { f = -f; translateZ = -translateZ; }
        if (flipX) { r = -r; translateX = -translateX; }
        if (flipY) { u = -u; translateY = -translateY; }

        _.m_data[0][0] = r.x; _.m_data[0][1] = u.x; _.m_data[0][2] = f.x; _.m_data[0][3] = 0.0f;
        _.m_data[1][0] = r.y; _.m_data[1][1] = u.y; _.m_data[1][2] = f.y; _.m_data[1][3] = 0.0f;
        _.m_data[2][0] = r.z; _.m_data[2][1] = u.z; _.m_data[2][2] = f.z; _.m_data[2][3] = 0.0f;
        _.m_data[3][0] = translateX; _.m_data[3][1] = translateY; _.m_data[3][2] = translateZ; _.m_data[3][3] = 1.0f;

        return _;
    }
    template <typename T>
    inline _mat4x4<T> perspective(T fov, T aspect, T nearClip = static_cast<T>(0.1), T farClip = static_cast<T>(1000)) {
        _mat4x4<T> _(static_cast<T>(0));

        T tanr = std::tan(degreesToRadians(fov) / static_cast<T>(2));
        T scale_y = static_cast<T>(1) / tanr;
        T scale_x = scale_y / aspect;

        _.m_data[0][0] = scale_x;
        _.m_data[1][1] = scale_y;
        _.m_data[2][2] = -(farClip + nearClip) / (farClip - nearClip);
        _.m_data[3][2] = -(static_cast<T>(2) * farClip * nearClip) / (farClip - nearClip);
        _.m_data[2][3] = static_cast<T>(-1);

        return _;
    }
    template <typename T> 
    inline _mat4x4<T> orthographic(T left, T right, T bottom, T top, T nearClip, T farClip) {
        _mat4x4<T> _;
        T cast_2 = static_cast<T>(2);

        _.m_data[0][0] = cast_2 / (right - left);
        _.m_data[1][1] = cast_2 / (top - bottom);
        _.m_data[2][2] =-cast_2 / (farClip - nearClip);

        _.m_data[3][0] =-(right + left) / (right - left);
        _.m_data[3][1] =-(top + bottom) / (top - bottom);
        _.m_data[3][2] =-(farClip + nearClip) / (farClip - nearClip);

        return _;
    }
    template <typename T>
    inline _vec3<T> _vec3<T>::transformed(const _mat4x4<T>& mat, bool isDirection) const {
        _vec3<T> _;

        float multiplier = (isDirection) ? 0.0f : 1.0f;

        _.x = x * mat.m_data[0][0] + y * mat.m_data[1][0] + z * mat.m_data[2][0] + multiplier * mat.m_data[3][0];
        _.y = x * mat.m_data[0][1] + y * mat.m_data[1][1] + z * mat.m_data[2][1] + multiplier * mat.m_data[3][1];
        _.z = x * mat.m_data[0][2] + y * mat.m_data[1][2] + z * mat.m_data[2][2] + multiplier * mat.m_data[3][2];
        
        return _;
    }
    template <typename T>
    inline _vec3<T> _vec3<T>::transform(const _mat4x4<T>& mat, bool isDirection) {
        *this = this->transformed(mat, isDirection);
        return *this;
    }
    template <typename T>
    inline _mat4x4<T> _mat4x4<T>::transposed() {
        _mat4x4<T> _;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                _.m_data[c][r] = m_data[r][c];
            }
        }
        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::transpose() {
        *this = this->transposed();
        return *this;
    }
    template <typename T>
    // https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
    inline _mat4x4decomp<T> _mat4x4<T>::decompose() {
        _mat4x4decomp<T> d;

        d.position.x = m_data[3][0];
        d.position.y = m_data[3][1];
        d.position.z = m_data[3][2];

        // upper-left columns
        T ULcolumns[3][3];

        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 3; j++) {
                ULcolumns[i][j] = m_data[i][j];;
            }
        }

        // helper function
        auto column = [&](size_t i) {
            return _vec3<T>{ULcolumns[i][0], ULcolumns[i][1], ULcolumns[i][2]};
        };

        d.scale.x = column(0).length(); // scale X
        for (size_t i = 0; i < 3; ++i) {
            ULcolumns[0][i] /= d.scale.x; 
        }

        d.skew.x = column(0).dot(column(1)); // skew XY
        for (size_t i = 0; i < 3; ++i) {
            ULcolumns[1][i] -= d.skew.x * ULcolumns[0][i];
        }

        d.scale.y = column(1).length(); // scale Y
        for (size_t i = 0; i < 3; ++i) {
            ULcolumns[1][i] /= d.scale.y; 
        }

        d.skew.y /= d.scale.y;

        d.skew.y = column(0).dot(column(2)); // skew XZ
        for (size_t i = 0; i < 3; ++i) {
            ULcolumns[2][i] -= d.skew.y * ULcolumns[0][i];
        }

        d.skew.z = column(1).dot(column(2)); // skew YZ
        for (size_t i = 0; i < 3; ++i) { 
            ULcolumns[2][i] -= d.skew.z * ULcolumns[1][i]; 
        }

        d.scale.z = column(2).length(); // scale Z
        for (size_t i = 0; i < 3; ++i) { 
            ULcolumns[2][i] /= d.scale.z; 
        }

        d.skew.y /= d.scale.z;
        d.skew.z /= d.scale.z;

        // check for a orientation flip or negative determinant
        T det = {
            ULcolumns[0][0] * (ULcolumns[1][1] * ULcolumns[2][2] - ULcolumns[1][2] * ULcolumns[2][1]) -
            ULcolumns[0][1] * (ULcolumns[1][0] * ULcolumns[2][2] - ULcolumns[1][2] * ULcolumns[2][0]) +
            ULcolumns[0][2] * (ULcolumns[1][0] * ULcolumns[2][1] - ULcolumns[1][1] * ULcolumns[2][0]) 
        };
        if (det < 0) {
            d.scale.z *= -1.0; // flip z
            for (size_t i = 0; i < 3; ++i) {
                ULcolumns[2][i] *= -1.0;
            }
        }

        d.rotation = _mat4x4<T>(
            ULcolumns[0][0], ULcolumns[0][1], ULcolumns[0][2], 0,
            ULcolumns[1][0], ULcolumns[1][1], ULcolumns[1][2], 0,
            ULcolumns[2][0], ULcolumns[2][1], ULcolumns[2][2], 0,
            0, 0, 0, 0
        ).extractEuler();
        return d;
    }
    template <typename T>
    inline _vec3<T> _mat4x4<T>::extractEuler() {
        _vec3<T> v;
    
        T r20 = m_data[2][0]; 

        // check for gimbal lock and protect against it
        if (std::abs(r20) < 1.0 - 0.0001) {
            v.x = -std::asin(r20);
            v.y = std::atan2(m_data[2][1], m_data[2][2]);
            v.z = std::atan2(m_data[1][0], m_data[0][0]);
        } else {
            // gimbal lock triggered, lock to +-90
            v.x = (r20 < 0) ? static_cast<T>(M_PI_2) : static_cast<T>(-M_PI_2);
            v.y = 0.0;
            v.z = std::atan2(-m_data[0][1], m_data[1][1]);
        }

        return {fm::radiansToDegrees(v.x), fm::radiansToDegrees(v.y), fm::radiansToDegrees(v.z)};
    }
    template <typename T>
    // https://en.wikipedia.org/wiki/Cramer%27s_rule#:~:text=Finding%20inverse%20matrixedit
    // https://www.scribd.com/document/869851943/Investigatory-Project
    inline _mat4x4<T> _mat4x4<T>::inversed() {
        _mat4x4<T> _;

        // adj(A) = adj(A)A = det(A)I (adjoint)

        // sd = subdeterminants
        T sd0 = m_data[0][0] * m_data[1][1] - m_data[1][0] * m_data[0][1];
        T sd1 = m_data[0][0] * m_data[1][2] - m_data[1][0] * m_data[0][2];
        T sd2 = m_data[0][0] * m_data[1][3] - m_data[1][0] * m_data[0][3];
        T sd3 = m_data[0][1] * m_data[1][2] - m_data[1][1] * m_data[0][2];
        T sd4 = m_data[0][1] * m_data[1][3] - m_data[1][1] * m_data[0][3];
        T sd5 = m_data[0][2] * m_data[1][3] - m_data[1][2] * m_data[0][3];

        T c0 = m_data[2][0] * m_data[3][1] - m_data[3][0] * m_data[2][1];
        T c1 = m_data[2][0] * m_data[3][2] - m_data[3][0] * m_data[2][2];
        T c2 = m_data[2][0] * m_data[3][3] - m_data[3][0] * m_data[2][3];
        T c3 = m_data[2][1] * m_data[3][2] - m_data[3][1] * m_data[2][2];
        T c4 = m_data[2][1] * m_data[3][3] - m_data[3][1] * m_data[2][3];
        T c5 = m_data[2][2] * m_data[3][3] - m_data[3][2] * m_data[2][3];

        T det = sd0 * c5 - sd1 * c4 + sd2 * c3 + sd3 * c2 - sd4 * c1 + sd5 * c0;

        // have you ever heard the phrase "fuck it we ball"?
        // i am typing in random things i have found from random websites nobody has ever heard of
        // i do not know what i am writing i barely know what an adjugate is it is 4 am

        if (std::abs(det) < 1e-6) return _mat4x4<T>::identity(); // don't divide by zero 
        T invDet = static_cast<T>(1.0) / det;

        // adjoint shit multiplied by the inverse determinindwaikntf
        _.m_data[0][0] = ( m_data[1][1] * c5  - m_data[1][2] * c4  + m_data[1][3] * c3 ) * invDet;
        _.m_data[0][1] = (-m_data[0][1] * c5  + m_data[0][2] * c4  - m_data[0][3] * c3 ) * invDet;
        _.m_data[0][2] = ( m_data[3][1] * sd5 - m_data[3][2] * sd4 + m_data[3][3] * sd3) * invDet;
        _.m_data[0][3] = (-m_data[2][1] * sd5 + m_data[2][2] * sd4 - m_data[2][3] * sd3) * invDet;

        _.m_data[1][0] = (-m_data[1][0] * c5  + m_data[1][2] * c2  - m_data[1][3] * c1 ) * invDet;
        _.m_data[1][1] = ( m_data[0][0] * c5  - m_data[0][2] * c2  + m_data[0][3] * c1 ) * invDet;
        _.m_data[1][2] = (-m_data[3][0] * sd5 + m_data[3][2] * sd2 - m_data[3][3] * sd1) * invDet;
        _.m_data[1][3] = ( m_data[2][0] * sd5 - m_data[2][2] * sd2 + m_data[2][3] * sd1) * invDet;

        _.m_data[2][0] = ( m_data[1][0] * c4  - m_data[1][1] * c2  + m_data[1][3] * c0 ) * invDet;
        _.m_data[2][1] = (-m_data[0][0] * c4  + m_data[0][1] * c2  - m_data[0][3] * c0 ) * invDet;
        _.m_data[2][2] = ( m_data[3][0] * sd4 - m_data[3][1] * sd2 + m_data[3][3] * sd0) * invDet;
        _.m_data[2][3] = (-m_data[2][0] * sd4 + m_data[2][1] * sd2 - m_data[2][3] * sd0) * invDet;

        _.m_data[3][0] = (-m_data[1][0] * c3  + m_data[1][1] * c1  - m_data[1][2] * c0 ) * invDet;
        _.m_data[3][1] = ( m_data[0][0] * c3  - m_data[0][1] * c1  + m_data[0][2] * c0 ) * invDet;
        _.m_data[3][2] = (-m_data[3][0] * sd3 + m_data[3][1] * sd1 - m_data[3][2] * sd0) * invDet;
        _.m_data[3][3] = ( m_data[2][0] * sd3 - m_data[2][1] * sd1 + m_data[2][2] * sd0) * invDet;

        return _;
    }
    template <typename T>
    inline _mat4x4<T>& _mat4x4<T>::inverse() {
        *this = this->inversed();
        return *this;
    }

    using mat4  = _mat4x4<float>;
    using imat4 = _mat4x4<bool>;
    using dmat4 = _mat4x4<double>;
    using bmat4 = _mat4x4<bool>;
    using lmat4 = _mat4x4<long>;
    using smat4 = _mat4x4<short>; 
}