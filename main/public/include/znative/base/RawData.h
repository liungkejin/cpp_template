//
// Created by LiangKeJin on 2024/11/10.
//

#pragma once

#include <znative/ZBase.h>

NAMESPACE_DEFAULT

class RawData {
public:
    RawData() = default;

    explicit RawData(size_t size) : m_size(size) {
        m_data = size > 0 ? std::shared_ptr<uint8_t[]>(new uint8_t[size], [](uint8_t *p) {
            // _INFO("delete created data: %p", p);
            delete[] p;
        }): nullptr;
        // _INFO("Create RawData(size_t size): %p, size(%d)", m_data.get(), size);
    }

    RawData(uint8_t *data, size_t size, bool owned = false) : m_size(size) {
        if (owned) {
            // _INFO("take ownership of data: %p, size: %d", data, size);
            m_data = std::shared_ptr<uint8_t[]>(data, [](uint8_t *p) {
                // _INFO("delete owned data: %p", p);
                delete[] p;
            });
        } else {
            // _INFO("wrap data: %p, size: %d", data, size);
            m_data = std::shared_ptr<uint8_t[]>(data, [](uint8_t *p) {
                // _INFO("skip delete data: %p", p);
            });
        }
    }

    RawData(const RawData& o) = default;

    RawData & operator=(const RawData& o) = default;

    // 返回当前缓冲区的子视图，不拷贝底层数据。
    // 返回值与原 RawData 共享同一块内存，调用方需要保证原数据生命周期覆盖该视图。
    [[nodiscard]] RawData slice(size_t offset, size_t size) const {
        if (offset > m_size || size > (m_size - offset)) {
            return {};
        }
        if (size == 0) {
            return {};
        }
        if (!m_data) {
            return {};
        }
        return RawData(std::shared_ptr<uint8_t[]>(m_data, m_data.get() + offset), size);
    }

public:
    [[nodiscard]] const uint8_t *data() const { return m_data.get(); }

    uint8_t *data() { return m_data.get(); }

    [[nodiscard]] size_t size() const { return m_size; }

    std::string toString() {
        return {reinterpret_cast<const char *>(m_data.get()), m_size};
    }
private:
    RawData(std::shared_ptr<uint8_t[]> data, size_t size) : m_data(std::move(data)), m_size(size) {}

    // uint8_t * m_data = nullptr;
    std::shared_ptr<uint8_t[]> m_data = nullptr;
    size_t m_size = 0;
};

NAMESPACE_END
