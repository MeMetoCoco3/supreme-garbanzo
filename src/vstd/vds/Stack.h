#include <cstddef>
#include <cstdio>
#include "vlogger.h"

namespace vds {
    
    template<typename T>
    struct Stack {
        private: 
            static constexpr size_t m_capacity = 64;
            // Alignas align the next buffer as type T
            alignas(T) std::byte m_buffer[m_capacity * sizeof(T)];
            size_t m_top = 0;
            T* slot(size_t i) {
                return reinterpret_cast<T*>(m_buffer + sizeof(T) * i);
            }

        public:
            ~Stack(){
                while(m_top > 0){
                    m_top--;
                    slot(m_top)->~T();
                }
            }

            void push(T val) {
                if (m_top >= m_capacity) {
                    V_LOG_ERROR("Capacity on stack exceded.");
                    return;
                }
                // PLACEMENT NEW, it does not allocate in the heap, it uses the memory provided, and calls constructor on it.
                new (slot(m_top)) T(std::move(val));
                m_top++;
            };

            T pull(){
                if (m_top == 0) {
                    V_LOG_ERROR("Stack is empty.");
                    return {};
                }
                m_top -=1 ;
                T val = std::move(*slot(m_top));
                slot(m_top)->~T();
                return val;
            };

            T peek() {
                if (m_top == 0) {
                    V_LOG_ERROR("Stack is empty.");
                    return {};
                }

                return *slot(m_top-1);
            }

            size_t size() const { return m_top; };
            bool empty() const { return m_top == 0; };
    };
}
