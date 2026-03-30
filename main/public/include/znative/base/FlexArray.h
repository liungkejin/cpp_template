//
// Created on 2024/6/30.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".


#pragma once

#include <znative/ZBase.h>
#include <znative/ZLog.h>

NAMESPACE_DEFAULT

template<typename T>
class FlexArray {
public:
    explicit FlexArray(int initCap = 0) {
        if (initCap > 0) {
            this->obtain(initCap);
        }
    }

    FlexArray(const FlexArray &other) : m_data(other.m_data),
                                        m_data_size(other.m_data_size), m_capacity(other.m_capacity) {
    }

    FlexArray &operator=(const FlexArray &other) {
        m_data = other.m_data;
        m_data_size = other.m_data_size;
        m_capacity = other.m_capacity;
        return *this;
    }

    T &operator[](size_t index) {
        _FATAL_IF(index >= m_data_size, "Invalid index(%d) array size: %d", index, m_data_size);
        return m_data[index];
    }

public:
    bool empty() const { return m_data_size == 0 || m_data == nullptr; }

    size_t size() const { return m_data_size; }

    size_t capacity() const { return m_capacity; }

    /**
     * memset 数组
     * @param value 填充值
     * @param offset 偏移量
     * @param size 大小 -1 表示填充到数组末尾
     * @return 数组0位指针
     */
    T* memset(int value = 0, int offset = 0, int size = -1) {
        if (m_data) {
            if (offset < 0) {
                offset = 0;
            }
            if (size == -1 || size+offset > m_capacity) {
                size = m_capacity - offset;
            }
            std::memset(this->data(offset), value, size * sizeof(T));
        }
        return this->data();
    }

    /**
     * 填充数组
     * @param value 填充值
     * @param offset 偏移量
     * @param size 大小 -1 表示填充到数组末尾
     * @return 数组0位指针
     */
    T* fill(T value, int offset = 0, int size = -1) {
        if (offset < 0) {
            offset = 0;
        }
        int cap = m_capacity;
        if (size == -1 || size > cap) {
            size = cap;
        }

        T *d = this->data(0);
        if (d) {
            for (int i = offset; i < size; i++) {
                d[i] = value;
            }
        }
        return this->data();
    }

    /**
     * 重置为 init 值
     * @param size
     * @param init
     * @return
     */
    T *obtainInit(size_t size, int init = 0) {
        T *data = obtain(size);
        if (data) {
            std::memset(data, init, size * sizeof(T));
        }
        return data;
    }

    T *obtain(size_t size, bool keepData = false) {
        if (size > m_capacity) {
            m_capacity = size;
            std::shared_ptr<T> ndata(new T[size], std::default_delete<T[]>());
            if (keepData) {
                if (m_data && m_data_size > 0) {
                    memcpy((uint8_t *) ndata.get(), (uint8_t *) m_data.get(), m_data_size * sizeof(T));
                }
            }
            m_data = ndata;
        }
        m_data_size = size;
        return m_data.get();
    }

    T *put(const T *src, size_t size, bool keepData = false) {
        uint8_t *dst = (uint8_t *) obtain(size, keepData);
        if (src && (uint8_t *)src != dst) {
            size_t unitSize = sizeof(T);
            memcpy(dst, (uint8_t *) src, size * unitSize);
        }
        return m_data.get();
    }

    void copyTo(FlexArray &other) {
        other.obtain(m_data_size);
        memcpy(other.m_data.get(), m_data.get(), m_data_size * sizeof(T));
    }

    T *data(int index = 0) {
        if (index < 0) {
            return nullptr;
        }
        if (index >= m_data_size) {
            return nullptr;
        }
        return ((T *)m_data.get()) + index * sizeof(T);
    }

    const T *data(int index = 0) const { return ((const T *)m_data.get()) + index * sizeof(T); }

    uint8_t *bytes(int index = 0) { return (uint8_t *) data(index); }
    
    const uint8_t *bytes(int index = 0) const { return (const uint8_t *) data(index); }

    void free() {
        m_data = nullptr;
        m_data_size = 0;
        m_capacity = 0;
    }

private:
    std::shared_ptr<T> m_data = nullptr;
    size_t m_data_size = 0;
    size_t m_capacity = 0;
};

typedef FlexArray<char> ByteArray;
typedef FlexArray<unsigned char> UByteArray;
typedef FlexArray<int> IntArray;
typedef FlexArray<unsigned int> UIntArray;
typedef FlexArray<long long> LongArray;
typedef FlexArray<unsigned long long> ULongArray;
typedef FlexArray<float> FloatArray;
typedef FlexArray<double> DoubleArray;

class FlexByteArray {
public:
    explicit FlexByteArray(int initCapBytes = 0) : m_data(initCapBytes) {
    }

    FlexByteArray(const FlexByteArray &other) : m_data(other.m_data) {
    }

    FlexByteArray &operator=(const FlexByteArray &other) {
        m_data = other.m_data;
        return *this;
    }

    template<typename T>
    T &operator[](size_t index) {
        return *this->data<T>(index);
    }

public:
    bool empty() const { return m_data.empty(); }

    template<typename T>
    size_t size() const {
        return m_data.size() / sizeof(T);
    }

    size_t sizeBytes() const {
        return m_data.size();
    }

    template<typename T>
    size_t capacity() const {
        return m_data.capacity() / sizeof(T);
    }

    size_t capacityBytes() const {
        return m_data.capacity();
    }

    /**
     * memset 数组
     * @param value 填充值
     * @param offset 偏移量
     * @param size 大小 -1 表示填充到数组末尾
     * @return 数组0位指针
     */
    template<typename T>
    T *memset(int value = 0, int offset = 0, int size = -1) {
        m_data.memset(value, offset, size);
        return this->data<T>();
    }

    /**
     * 填充数组
     * @param value 填充值
     * @param offset 偏移量
     * @param size 大小 -1 表示填充到数组末尾
     * @return 数组0位指针
     */
    template<typename T>
    T *fill(T value, int offset = 0, int size = -1) {
        if (offset < 0) {
            offset = 0;
        }
        T *d = this->data<T>();
        int cap = this->capacity<T>();
        if (size == -1 || size > cap) {
            size = cap;
        }
        if (d) {
            for (int i = offset; i < size; i++) {
                d[i] = value;
            }
        }
        return this->data<T>();
    }

    template<typename T>
    T *obtain(size_t size, bool keepData = false) {
        size_t needSize = size * sizeof(T);
        return (T *) m_data.obtain(needSize, keepData);
    }

    template<typename T>
    T *put(const T *src, size_t size, bool keepData = false) {
        size_t needSize = size * sizeof(T);
        uint8_t *dst = m_data.obtain(needSize, keepData);
        if (src) {
            memcpy(dst, (const uint8_t *) src, needSize);
        }
        return (T *) dst;
    }

    template<typename T>
    T *data(int index = 0) {
        index *= sizeof(T);
        return (T *) m_data.data(index);
    }

    template<typename T>
    T& at(int index = 0) {
        return *data<T>(index);
    }

    uint8_t *bytes(int index = 0) {
        return m_data.bytes() + index;
    }

    void free() {
        m_data.free();
    }

private:
    UByteArray m_data;
};


NAMESPACE_END
