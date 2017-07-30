#ifndef REACTIVE_V3_DELEGATETAG_H
#define REACTIVE_V3_DELEGATETAG_H

#include <atomic>

namespace reactive {

    namespace details {
        namespace Delegate {
            // 0 - reserved for empty tag?
            static std::atomic<unsigned long long> delegate_uuid{ 1 };	// should be enough approx. for 1000 years at 3Ghz continuous incrementation
        }
    }

    // pass by copy
    class DelegateTag {
        unsigned long long tag;

    public:
        DelegateTag(std::nullptr_t)
            :tag(0)
        {}
        DelegateTag()
            : tag(details::Delegate::delegate_uuid.fetch_add(1))
        {}

        bool operator==(const DelegateTag& other) const {
            return tag == other.tag;
        }
        bool operator!=(const DelegateTag& other) const {
            return tag != other.tag;
        }
    };

}

#endif //REACTIVE_V3_DELEGATETAG_H
