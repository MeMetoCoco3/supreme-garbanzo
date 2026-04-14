#include "../vtypes.h"

#pragma once

namespace vds {
    template <typename T>
    struct s_DNode {
        T data;
        i32 unique_id = 0;
        s_DNode<T>* next;
        s_DNode<T>* prev;

        s_DNode(T val, int id, s_DNode<T>* next, s_DNode<T>* prev):
            data(val),
            unique_id(id),
            next(next), 
            prev(prev)
        {}
    };


    
    template <typename s_DLinkList>
    struct s_DLIterator {
        public: 
            using ValueType = typename s_DLinkList::ValueType;
            using NodeType = typename s_DLinkList::NodeType;
            using PointerType = NodeType*;
            using ReferenceType = NodeType&;

        public:
            s_DLIterator(PointerType ptr)
                : m_Ptr(ptr) {}
            
            s_DLIterator& operator++() {
                m_Ptr = m_Ptr->next;
                return *this;
            }
            
            // This is the possfix operator
            s_DLIterator operator++(int) {
                s_DLIterator iterator = *this;
                ++(*this);
                return iterator;
            }

            s_DLIterator& operator--() {
                m_Ptr = m_Ptr->prev;
                return *this;
            }
            
            s_DLIterator operator--(int) {
                s_DLIterator iterator = *this;
                --(*this);
                return iterator;
            }

            // ReferenceType operator[](int index)
            // {
            //     return *(m_Ptr + index);
            // }

            PointerType operator->()  const {
                return m_Ptr;
            }   

            ReferenceType operator*() const {
                return *m_Ptr;
            }

            bool operator == (const s_DLIterator& other ) const {
                return m_Ptr == other.m_Ptr;
            }

            bool operator != (const s_DLIterator& other ) const {
                return m_Ptr != other.m_Ptr;
            }

        private: 
            NodeType* m_Ptr;

        
    };


    template <typename T>
    struct DLinkedList {
        public: 
            using ValueType = T;
            using NodeType = s_DNode<T>;
            using NodePtr = NodeType*;
            using Iterator = s_DLIterator<DLinkedList<T>>;
        private: 
            i32 m_id_count = 0;
            i32 m_count = 0;
        public: 
            DLinkedList() = default;
            ~DLinkedList() = default;

            NodePtr head = nullptr;
            NodePtr tail = nullptr;
            

            Iterator begin() const {
                return Iterator(head);
            };

            Iterator end() const {
                return Iterator(nullptr);
            };


            i32 append(T val){
                NodePtr new_node = new s_DNode<T>(val, m_id_count++, nullptr, tail);
                if(!head) {
                    head = new_node;
                } else {
                    new_node->prev = tail;
                    tail->next = new_node;
                } 

                tail = new_node;
                return new_node->unique_id;
            }
            
            i32 append_head(T val) {
                NodePtr new_node = new s_DNode<T>(val, m_id_count++, nullptr, nullptr);
                if(!head) {
                    tail = new_node;
                } else {
                    new_node->next = head;
                    head->prev = new_node;
                }

                head = new_node;
                return new_node->unique_id;
            }
            
            void swap_given_indexes(i32 a, i32 b)
            {
                auto node_a = get_given_idx(a);
                auto node_b = get_given_idx(b);
                swap(node_a, node_b);
            }

            void swap(NodePtr a, NodePtr b)
            {
                if (!a || !b || a->unique_id == b->unique_id)
                    return;

                auto* a_prev = a->prev;
                auto* a_next = a->next;
                auto* b_prev = b->prev;
                auto* b_next = b->next;
                
                if (!a_prev) head = b;
                if (!b_prev) head = a;

                if (!a_next) tail = b;
                if (!b_next) tail = a;
                
                if (a_prev) a_prev->next = b;
                if (a_next) a_next->prev = b;
                if (b_prev) b_prev->next = a;
                if (b_next) b_next->prev = a;

                // 3 cases 
                // 1. a <-> b
                if((a_next && a_next->unique_id == b->unique_id)){
                    a->prev = b;
                    a->next = b_next;
                    b->prev = a_prev;
                    b->next = a;
                } else 
                // 2. b <-> a
                if (b_next && b_next->unique_id == a->unique_id){
                    b->prev = a;
                    b->next = a_next;
                    a->prev = b_prev;
                    a->next = b;
                } else 
                // 3. Not adjacent
                {
                    a->prev = b_prev;
                    a->next = b_next;
                    b->prev = a_prev;
                    b->next = a_next;
                }
            }
            
            NodePtr get_given_id(i32 id){
                for(Iterator it = this->begin();
                        it != this->end(); ++it) {
                    if (it->data.unique_id == id)
                        return &(*it);
                }
                return nullptr;
            }

            NodePtr get_given_idx(i32 idx){
                i32 cnt = 0;
                for(Iterator it = this->begin();
                        it != this->end(); ++it) {
                    if (cnt == idx)
                        return &(*it);
                    cnt++;
                }
                return {};
            }
            bool remove_given_id(i32 id);
            bool remove_given_idx(i32 idx);
    };
};
