#pragma once
struct IObjectSharedState;
struct IObject;
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

        shared_ptr ( shared_ptr && other)
        {
            this->obj_state = other.obj_state;
            if(obj_state)
            {
                obj_state->IncrementState();
                obj_state->IncrementObject();
            }
        }

        shared_ptr(const shared_ptr& other)
        {
            this->obj_state = other.obj_state;
            if(obj_state)
            {
                obj_state->IncrementState();
                obj_state->IncrementObject();
            }
        }

        template<class U> shared_ptr(const shared_ptr<U>& other):
            obj_state(nullptr)
        {
            if(other.obj_state)
            {
                if(dynamic_cast<T*>(other.obj_state->GetObject()))
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
        void reset(IObject* obj = nullptr)
        {
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            if(obj)
            {
                obj_state = obj->GetConstructor()->GetState(obj->GetPerTypeId());
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }else
            {
                obj_state = nullptr;
            }
        }

        shared_ptr& operator=(const shared_ptr& other)
        {
            if(obj_state)
            {
                obj_state->DecrementObject();
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            if(obj_state)
            {
                obj_state->IncrementObject();
                obj_state->IncrementState();
            }
            return *this;
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
        
        T* Get()
        {
            return static_cast<T*>(obj_state->GetObject());
        }
		const T* Get() const
		{
			return static_cast<T*>(obj_state->GetObject());
		}
        
        T* operator->()
        {
            return Get();
        }

		const T* operator->() const
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
                return dynamic_cast<U*>(obj_state->GetObject());
            }
            return nullptr;
        }

		operator bool() const
		{
			return !empty();
		}

        bool operator==(T* p)
        {
            if(obj_state)
            {
                return obj_state->GetObject() == p;
            }
            return false;
        }
        bool operator != (T* p)
        {
            if(obj_state)
            {
                return obj_state->GetObject() != p;
            }
            return true;
        }
        bool operator == (shared_ptr const & r)
        {
            return r.obj_state == obj_state;
        }
        bool operator != (shared_ptr const& r)
        {
            return r.obj_state != obj_state;
        }
        
        template<class U> U* StaticCast()
        {
            if(obj_state)
            {
                return static_cast<T*>(obj_state->GetObject());
            }
            return nullptr;
        }
        
        explicit operator T*()
        {
            return Get();
        }

        IObjectSharedState* GetState() const
        {
            return obj_state;
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
        
        weak_ptr ( weak_ptr && other)
        {
            this->obj_state = other.obj_state;
            if(obj_state)
                obj_state->IncrementState();
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
            if(dynamic_cast<T*>(state->GetObject()))
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

        weak_ptr & operator= ( weak_ptr && other)
        {
            if(obj_state)
            {
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            if(obj_state)
            {
                obj_state->IncrementState();
            }
            return *this;
        }

        weak_ptr& operator=(const weak_ptr& other)
        {
            if(obj_state)
            {
                obj_state->DecrementState();
            }
            obj_state = other.obj_state;
            if(obj_state)
            {
                obj_state->IncrementState();
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

        explicit operator T*()
        {
            return Get();
        }

        IObjectSharedState* GetState() const
        {
            return obj_state;
        }

    private:
        IObjectSharedState* obj_state;
    };
}