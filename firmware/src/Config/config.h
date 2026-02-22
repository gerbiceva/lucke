#include <vector>
#include <string>
#include <memory>
#include <cstring>
#include <cstddef>
#include <cstdint>

struct Serializable {
    virtual ~Serializable() = default;

    virtual void* serialize(void* dst) const = 0;
    virtual size_t serializationSize() const = 0;
};

struct Setting : Serializable {
    Setting(std::string name) : name(name) {}

    std::string name;

    void* serialize(void* dst) const override {
        // Write string length first
        uint32_t len = static_cast<uint32_t>(name.size());
        std::memcpy(dst, &len, sizeof(len));
        dst += sizeof(len);

        std::memcpy(dst, name.data(), name.size());
        return dst + name.size();
    }

    size_t serializationSize() const override {
        return sizeof(uint32_t) + name.size();
    }
};

template <typename TType>
struct TypedSetting : Setting {
    TypedSetting(const std::string& name, TType v)
        : Setting(name), value(v) {}

    TType value;

    void* serialize(void* dst) const override {
        void* running_dst = Setting::serialize(dst);

        // Safe write (no alignment UB)
        std::memcpy(running_dst, &value, sizeof(TType));
        return running_dst + sizeof(TType);
    }

    size_t serializationSize() const override {
        return Setting::serializationSize() + sizeof(TType);
    }
};

struct GroupConfig : Serializable {
    GroupConfig(std::string name, std::vector<std::shared_ptr<Setting>> settings) :
        name(std::move(name)), settings(std::move(settings)) {}

    std::string name;
    std::vector<std::shared_ptr<Setting>> settings;

    void* serialize(void* dst) const override {
        uint32_t len = static_cast<uint32_t>(name.size());
        std::memcpy(dst, &len, sizeof(len));
        dst += sizeof(len);

        std::memcpy(dst, name.data(), name.size());
        dst += name.size();

        for (const auto& setting : settings) {
            dst = setting->serialize(dst);
        }

        return dst;
    }

    size_t serializationSize() const override {
        size_t total_size = sizeof(uint32_t) + name.size();

        for (const auto& setting : settings) {
            total_size += setting->serializationSize();
        }

        return total_size;
    }
};

struct Config : Serializable {
    Config(std::vector<GroupConfig> group_configs) : group_configs(std::move(group_configs)) {

    }

    std::vector<GroupConfig> group_configs;

    void* serialize(void* dst) const override {
        for (const auto& group : group_configs) {
            dst = group.serialize(dst);
        }
        return dst;
    }

    size_t serializationSize() const override {
        size_t total_size = 0;  // FIXED

        for (const auto& group : group_configs) {
            total_size += group.serializationSize();
        }

        return total_size;
    }
};
