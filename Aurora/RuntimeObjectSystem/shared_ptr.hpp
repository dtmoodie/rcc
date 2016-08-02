#pragma once
struct IObjectSharedState;
namespace rcc
{
    template<class T> class weak_ptr;
    template<class T> class shared_ptr
    {
    public:
        shared_ptr():
            obj_state(nullptr)
        {
        
        }
        shared_ptr(IObject* obj):
            obj_state(nullptr)
        {
            if(dynamic_cast<T*>(obj))
            {
                obj_state = obj->GetConstructor()->GetState(obj->GetPerTypeId());
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }
        }
        shared_ptr(IObjectSharedState* state)
        {
            T* obj = dynamic_cast<T*>(state->GetObject());
            if(obj)
            {
                obj_state = state;
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }
        }
        template<class U> shared_ptr(const shared_ptr<U>& other)
        {
            if(dynamic_cast<T*>(other.obj_state->GetObject()))
            {
                obj_state = other.obj_state;
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }
        }
        
        template<class U> shared_ptr& operator=(const shared_ptr<U>& other)
        {
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            if(dynamic_cast<T*>(other.obj_state->GetObject()))
            {
                obj_state = other->obj_state;
                obj_state->IncrementObject();
                obj_state->IncrementState();
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
        T* Get()
        {
            return static_cast<T*>(obj_state->GetObject());
        }
        T* operator->()
        {
            return Get();
        }
        bool empty() const
        {
            if(obj_state)
            {
                return obj_state->GetObject() == nullptr;
            }
            return false;
        }
        template<class U> U* DynamicCast()
        {
            if(obj_state)
            {
                return dynamic_cast<T*>(obj_state->GetObject());
            }
            return nullptr;
        }
        template<class U> U* StaticCast()
        {
            if(obj_state)
            {
                return static_cast<T*>(obj_state->GetObject());
            }
            return nullptr;
        }
    private:
        template<class T> friend class shared_ptr;
        template<class T> friend class weak_ptr;
        IObjectSharedState* obj_state;
    };

    template< class T> class weak_ptr
    {
    public:
        weak_ptr():
            obj_state(nullptr)
        {

        }
        weak_ptr(T* obj)
        {
            obj_state = obj->GetConstructor()->GetState(obj->GetPerTypeId());
            obj_state->IncrementState();
        }
        weak_ptr(IObjectSharedState* state)
        {
            if(dynamic_cast<T*>(state->GetObject()))
            {
                obj_state = state;
                obj_state->IncrementState();
            }
        }
        template<class U> weak_ptr(const weak_ptr<U>& other)
        {
            if(other.obj_state)
            {
                if(dynamic_cast<T*>(other.obj_state->GetObject()))
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

        template<class U> weak_ptr& operator=(const weak_ptr<U>& other)
        {
            if(obj_state)
            {
                obj_state->DecrementState();
            }
            if(other.obj_state)
            {
                if(dynamic_cast<T*>(other.obj_state->GetObject()))
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
        T* Get()
        {
            return static_cast<T*>(obj_state->GetObject());
        }
        T* operator->()
        {
            return Get();
        }
        bool empty() const
        {
            if(obj_state)
            {
                return obj_state->GetObject() == nullptr;
            }
            return false;
        }
        template<class U> U* DynamicCast()
        {
            if(obj_state)
            {
                return dynamic_cast<T*>(obj_state->GetObject());
            }
            return nullptr;
        }
        template<class U> U* StaticCast()
        {
            if(obj_state)
            {
                return static_cast<T*>(obj_state->GetObject());
            }
            return nullptr;
        }
        shared_ptr<T> lock()
        {
            return shared_ptr<T>(obj_state);
        }
    private:
        IObjectSharedState* obj_state;
    };
}