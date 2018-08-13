#pragma once
#include "IObjectState.hpp"

namespace rcc
{
    template<class T> class weak_ptr;
    template<class T> class shared_ptr
    {
    public:
        typedef T element_type;

        static shared_ptr create()
        {
            return T::create();
        }

        shared_ptr():
            obj_state(nullptr),
            obj_pointer(nullptr)
        {

        }

        shared_ptr(IObject* obj):
            obj_state(nullptr),
            obj_pointer(nullptr)
        {
            if((obj_pointer = dynamic_cast<T*>(obj)))
            {
                obj_state = IObjectSharedState::Get(obj);
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }
        }

        shared_ptr(IObjectSharedState* state)
        {
            obj_pointer = dynamic_cast<T*>(state->GetIObject());
            if(obj_pointer)
            {
                obj_state = state;
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }
        }

        shared_ptr ( shared_ptr && other){
            this->obj_state = other.obj_state;
            if(obj_state)
                obj_pointer = dynamic_cast<T*>(this->obj_state->GetIObject());
            if(obj_state && obj_pointer){
                other.obj_pointer = nullptr;
                other.obj_state = nullptr;
            }
        }

        shared_ptr(const weak_ptr<T>& other){
            if(other.obj_state)
            {
                other.obj_state->IncrementState();
            }
            this->obj_state = other.obj_state;
            if (obj_state)
            {
                obj_pointer = dynamic_cast<T*>(this->obj_state->GetIObject());
            }
            if(obj_state && obj_pointer){
                obj_state->IncrementObject();
            }
        }

        shared_ptr(weak_ptr<T>&& other){
            this->obj_state = other.obj_state;
            if (obj_state)
            {
                obj_pointer = dynamic_cast<T*>(this->obj_state->GetIObject());
            }
            if(obj_state && obj_pointer){
                other.obj_state = nullptr;
                obj_state->IncrementObject();
            }
        }

        shared_ptr(const shared_ptr& other)
        {
            this->obj_state = other.obj_state;
            obj_pointer = other.obj_pointer; 
            if(obj_state && obj_pointer)
            {
                obj_state->IncrementState();
                obj_state->IncrementObject();
            }
        }

        template<class U> shared_ptr(const shared_ptr<U>& other):
            obj_state(nullptr),
            obj_pointer(nullptr)
        {
            if(other.obj_state)
            {
                if((obj_pointer = dynamic_cast<T*>(other.obj_state->GetIObject())))
                {
                    obj_state = other.obj_state;
                    obj_state->IncrementObject();
                    obj_state->IncrementState();
                }else
                {
                    obj_state = nullptr;
                }
            }else
            {
                obj_state = nullptr;
            }
        }

        ~shared_ptr()
        {
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
        }

        shared_ptr& operator=(const T* other){
            if(obj_state){
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            if(other){
                obj_state = IObjectSharedState::Get(other);
                if(obj_state){
                    obj_pointer = dynamic_cast<T*>(obj_state->GetIObject());
                    obj_state->IncrementObject();
                    obj_state->IncrementState();
                }
            }
            return *this;
        }

        shared_ptr& operator=(const shared_ptr<T>& other)
        {
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            if(obj_state)
            {
                if ((obj_pointer = other.obj_pointer))
                {
                    obj_state->IncrementObject();
                    obj_state->IncrementState();
                }else{
                    obj_state = nullptr;
                }
            }
            return *this;
        }

        shared_ptr& operator=(shared_ptr<T>&& other){
            if(obj_state){
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            if(obj_state){
                if((obj_pointer = dynamic_cast<T*>(this->obj_state->GetIObject()))){
                    other.obj_state = nullptr;
                    other.obj_pointer = nullptr;
                }else{
                    obj_state = nullptr;
                }
            }
            return *this;
        }

        /*!
         *  /param other (const shared_ptr<U>& if this is a shared_ptr<const T> and U == T
         */
        template<class U>
        shared_ptr& operator=(typename std::enable_if<std::is_same<
                              typename std::remove_const<T>::type,U>::value && std::is_const<T>::value,
                              const shared_ptr<U>&>::type other){
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            obj_pointer = dynamic_cast<T*>(other.obj_pointer);
            if(obj_state)
            {
                obj_state = other->obj_state;
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }

            return *this;
        }
        // if T is not const or if U is a different types but matches constness
        template<class U> shared_ptr& operator=(typename std::enable_if<!std::is_const<T>::value ||
                                                std::is_const<U>::value == std::is_const<T>::value, const shared_ptr<U>&>::type other)
        {
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            obj_pointer = dynamic_cast<T*>(other.obj_pointer);
            if(obj_state)
            {
                obj_state = other->obj_state;
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }

            return *this;
        }

        void reset(IObject* obj = NULL)
        {
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            if(obj)
            {
                obj_state = IObjectSharedState::Get(obj);
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }else
            {
                obj_state = nullptr;
            }
        }


        T& operator*(){
            return *this->get();
        }

        const T& operator*() const{
            return *this->get();
        }

        template<class U> shared_ptr& operator=(const T& obj){
            if(obj_state){
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            obj_state = IObjectSharedState::Get(&obj);
            if(obj_state){
                obj_pointer = dynamic_cast<T*>(obj_state->GetIObject());
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }
            return *this;
        }



        T* get()
        {
            if(obj_state)
            {
                if(obj_pointer == obj_state->GetIObject())
                {
                    return obj_pointer;
                }else
                {
                    obj_pointer = dynamic_cast<T*>(obj_state->GetIObject());
                    return obj_pointer;
                }
            }
            return nullptr;
        }

        T* get() const
        {
            if (obj_state)
            {
                if (obj_pointer == obj_state->GetIObject())
                {
                    return obj_pointer;
                }
                else
                {
                    return dynamic_cast<T*>(obj_state->GetIObject());;
                }
            }
            return nullptr;
        }

        T* operator->()
        {
            return get();
        }

        T* operator->() const
        {
            return get();
        }

        bool empty() const
        {
            if(obj_state)
            {
                return obj_state->GetIObject() == nullptr;
            }
            return true;
        }

        template<class U> U* DynamicCast()
        {
            if(obj_state)
            {
                return dynamic_cast<U*>(obj_state->GetIObject());
            }
            return nullptr;
        }

        operator bool() const
        {
            return !empty();
        }

        bool operator==(T* p) const{
            if(obj_state){
                return obj_state->GetIObject() == p;
            }
            if(p == nullptr && obj_state == nullptr)
                return true;
            return false;
        }

        bool operator != (T* p) const
        {
            if(obj_state)
            {
                return obj_state->GetIObject() != p;
            }
            if(p == 0)
                return false;
            return true;
        }

        bool operator == (shared_ptr const & r) const{
            return r.obj_state == obj_state;
        }

        bool operator != (shared_ptr const& r) const{
            return r.obj_state != obj_state;
        }

        template<class U> U* StaticCast()
        {
            if(obj_state)
            {
                return static_cast<T*>(obj_state->GetIObject());
            }
            return nullptr;
        }

        explicit operator T*()
        {
            return get();
        }

        IObjectSharedState* GetState() const
        {
            return obj_state;
        }

private:
        template<class U> friend class shared_ptr;
        template<class U> friend class weak_ptr;
        IObjectSharedState* obj_state = nullptr;
        T* obj_pointer = nullptr;
    };



    template< class T> class weak_ptr
    {
    public:
        weak_ptr():
            obj_state(nullptr)
        {
        }

        weak_ptr ( weak_ptr && other)
        {
            this->obj_state = other.obj_state;
            if(obj_state)
                obj_state->IncrementState();
        }

        weak_ptr( shared_ptr<T>&& other){
            this->obj_state = other.obj_state;
            if(obj_state){
                other.obj_pointer = nullptr;
                other.obj_state = nullptr;
                obj_state->DecrementObject();
            }
        }

        weak_ptr(const weak_ptr & other)
        {
            this->obj_state = other.obj_state;
            if (obj_state)
                obj_state->IncrementState();
        }

        weak_ptr(T* obj)
        {
            if(obj)
            {
                obj_state = obj->GetConstructor()->GetState(obj->GetPerTypeId());
                obj_state->IncrementState();
            }else
            {
                obj_state = nullptr;
            }
        }

        weak_ptr(IObjectSharedState* state)
        {
            if(dynamic_cast<T*>(state->GetIObject()))
            {
                obj_state = state;
                obj_state->IncrementState();
            }else
            {
                obj_state = nullptr;
            }
        }

        template<class U> weak_ptr(const shared_ptr<U>& other)
        {
            if(other.obj_state)
            {
                if(dynamic_cast<T*>(other.obj_state->GetIObject()))
                {
                    obj_state = other.obj_state;
                    obj_state->IncrementState();
                }else{
                    obj_state = nullptr;
                }
            }else
            {
                obj_state = nullptr;
            }
        }

        template<class U> weak_ptr(const weak_ptr<U>& other)
        {
            if(other.obj_state)
            {
                if(dynamic_cast<T*>(other.obj_state->GetIObject()))
                {
                    obj_state = other.obj_state;
                    obj_state->IncrementState();
                }else
                {
                    obj_state = nullptr;
                }
            }else
            {
                obj_state = nullptr;
            }
        }

        weak_ptr & operator= ( weak_ptr && other){
            if(obj_state){
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            if(obj_state){
                other.obj_state = nullptr;
            }
            return *this;
        }

        weak_ptr& operator=(const weak_ptr& other){
            if(obj_state){
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            if(obj_state){
                obj_state->IncrementState();
            }
            return *this;
        }
        weak_ptr& operator=(const T* other){
            if(obj_state){
                obj_state->DecrementState();
            }
            if(other){
                obj_state = IObjectSharedState::Get(other);
                if(obj_state){
                    obj_state->IncrementState();
                }
            }
            return *this;
        }
        template<class U> weak_ptr& operator=(const weak_ptr<U>& other)
        {
            if(obj_state)
            {
                obj_state->DecrementState();
            }
            if(other.obj_state)
            {
                if(dynamic_cast<T*>(other.obj_state->GetIObject()))
                {
                    obj_state = other.obj_state;
                    obj_state->IncrementState();
                }else
                {
                    obj_state = nullptr;
                }
            }else
            {
                obj_state = nullptr;
            }

            return *this;
        }

        ~weak_ptr()
        {
            if(obj_state)
            {
                obj_state->DecrementState();
            }
        }

        void reset(IObject* obj = NULL)
        {
            if (obj_state)
            {
                obj_state->DecrementState();
            }
            if (obj)
            {
                obj_state = IObjectSharedState::Get(obj);
                obj_state->IncrementState();
            }
            else
            {
                obj_state = nullptr;
            }
        }

        T* get()
        {
            if(obj_state)
                return dynamic_cast<T*>(obj_state->GetIObject());
            return nullptr;
        }
        T* get() const
        {
            if(obj_state)
                return dynamic_cast<T*>(obj_state->GetIObject());
            return nullptr;
        }

        T& operator*(){
            return *this->get();
        }

        T& operator*() const{
            return *this->get();
        }

        T* operator->()
        {
            return get();
        }

        T* operator->() const
        {
            return get();
        }

        bool empty() const
        {
            if(obj_state)
            {
                return obj_state->GetIObject() == nullptr;
            }
            return true;
        }

        template<class U> U* DynamicCast() const
        {
            if(obj_state)
            {
                return dynamic_cast<U*>(obj_state->GetIObject());
            }
            return nullptr;
        }

        template<class U> U* StaticCast() const
        {
            if(obj_state)
            {
                return static_cast<T*>(obj_state->GetIObject());
            }
            return nullptr;
        }

        shared_ptr<T> lock()
        {
            return shared_ptr<T>(obj_state);
        }

        explicit operator T*()
        {
            return get();
        }

        IObjectSharedState* GetState() const
        {
            return obj_state;
        }

        bool operator==(T* p)
        {
            if(obj_state != nullptr)
            {
                return obj_state->GetIObject() == p;
            }else
            {
                if((p == nullptr) && (obj_state == nullptr))
                {
                    return true;
                }
            }
            return false;
        }

        bool operator != (T* p)
        {
            return !((*this) == p);
        }
        bool operator == (weak_ptr const & r)
        {
            return r.obj_state == obj_state;
        }
        bool operator != (weak_ptr const& r)
        {
            return r.obj_state != obj_state;
        }
        operator bool() const
        {
            return !empty();
        }

    private:
        template<class U> friend class shared_ptr;
        template<class U> friend class weak_ptr;
        IObjectSharedState* obj_state = nullptr;
    };
}
