#include <RuntimeCompiler/ICompilerLogger.h>
#include <RuntimeObjectSystem/IObject.h>
#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <RuntimeObjectSystem/RuntimeObjectSystem.h>

#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string>

bool destroyed = false;

class TestBaseObj: public IObject
{
public:
    using ParentClass = IObject;

    ~TestBaseObj()
    {
        destroyed = true;
    }

    int foo()
    {
        return 20;
    }
};

class TestDerivedObj: public TestBaseObj
{
public:
    using ParentClass = TestBaseObj;

    int foo()
    {
        return 10;
    }
};

REGISTERCLASS(TestBaseObj);
REGISTERCLASS(TestDerivedObj);

struct Fixture
{
    Fixture()
    {
        
    }

    ~Fixture()
    {
        assert(destroyed == true);
        destroyed = false;
    }
};

#define TEST_CASE(NAME) struct Fixture_##NAME: public Fixture{ \
    Fixture_##NAME():\
      Fixture(){} \
    void testCase()

#define END_TEST_CASE  };

IObjectConstructor* base_ctr = nullptr;
IObjectConstructor* derived_ctr = nullptr;

TEST_CASE(casting)
{
    TObjectControlBlock<TestDerivedObj> obj(nullptr);
    TObjectControlBlock<TestBaseObj>* ptr = &obj;
    auto derived = dynamic_cast<TObjectControlBlock<TestDerivedObj>*>(ptr);
    assert(derived);
    destroyed = true;
}END_TEST_CASE

TEST_CASE(construct)
{
    auto ctrs = PerModuleInterface::GetInstance()->GetConstructors();
    for(auto ct : ctrs)
    {
        if(std::string("TestBaseObj") ==  ct->GetName())
        {
            base_ctr = ct;
        }
        if(std::string("TestDerivedObj") == ct->GetName())
        {
            derived_ctr = ct;
        }
    }

    assert(derived_ctr != nullptr);
    assert(base_ctr != nullptr);
    {
        auto obj = base_ctr->Construct();
        assert(obj);
        assert(destroyed == false);
    }
    
}END_TEST_CASE

TEST_CASE(typed_pointer)
{
    {
        auto obj = base_ctr->Construct();
        assert(obj);
        rcc::shared_ptr<TestBaseObj> typed(obj);
        assert(typed);
        assert(typed->foo() == 20);
    }
}END_TEST_CASE

TEST_CASE(derived_pointer)
{
    {
        auto obj = derived_ctr->Construct();
        assert(obj);
        rcc::shared_ptr<TestBaseObj> base_ptr(obj);
        assert(base_ptr);
        assert(base_ptr->foo() == 20);
        rcc::shared_ptr<TestDerivedObj> derived_ptr(base_ptr);
        assert(derived_ptr);
        assert(derived_ptr->foo() == 10);
    }

} END_TEST_CASE


/*TEST_CASE(ctr_strong_from_strong){
    rcc::shared_ptr<TestSharedPtrObj> ptr(this->ptr);
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

TEST_CASE(ctr_weak_from_strong){
    rcc::weak_ptr<TestSharedPtrObj> ptr(this->ptr);
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

TEST_CASE(ctr_weak_from_state){
    rcc::weak_ptr<TestSharedPtrObj> ptr(state);
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

TEST_CASE(ctr_weak_from_weak){
    rcc::weak_ptr<TestSharedPtrObj> ptr(this->weak);
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

// Assignment operators
TEST_CASE(ass_strong_from_weak){
    rcc::shared_ptr<TestSharedPtrObj> ptr;
    ptr = weak;
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
}END_TEST_CASE

TEST_CASE(ass_strong_from_strong){
    rcc::shared_ptr<TestSharedPtrObj> ptr;
    ptr = this->ptr;
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

TEST_CASE(ass_weak_from_strong){
    rcc::weak_ptr<TestSharedPtrObj> ptr;
    ptr = this->ptr;
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

TEST_CASE(ass_weak_from_weak){
    rcc::weak_ptr<TestSharedPtrObj> ptr;
    ptr = this->weak;
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

// move operators
TEST_CASE(move_strong_from_weak){
    rcc::shared_ptr<TestSharedPtrObj> ptr;
    ptr = std::move(weak);
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count);
    //start_obj_count = 1;
    start_state_count -= 1;
}END_TEST_CASE

TEST_CASE(move_strong_from_strong){
    rcc::shared_ptr<TestSharedPtrObj> ptr;
    ptr = std::move(this->ptr);
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count);
    start_obj_count -= 1;
    start_state_count -= 1;
} END_TEST_CASE

TEST_CASE(move_weak_from_strong){
    rcc::weak_ptr<TestSharedPtrObj> ptr;
    ptr = std::move(this->ptr);
    assert(state->ObjectCount() == start_obj_count - 1);
    assert(state->StateCount() == start_state_count);
    start_obj_count -= 1;
    start_state_count -= 1;
} END_TEST_CASE

TEST_CASE(move_weak_from_weak){
    rcc::weak_ptr<TestSharedPtrObj> ptr;
    ptr = std::move(this->weak);
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count);
    start_state_count -= 1;
} END_TEST_CASE

// ReAssignment operators
TEST_CASE(reass_strong_from_weak){
    rcc::shared_ptr<TestSharedPtrObj> ptr(state);
    ptr = weak;
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
}END_TEST_CASE

TEST_CASE(reass_strong_from_strong){
    rcc::shared_ptr<TestSharedPtrObj> ptr(state);
    ptr = this->ptr;
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

TEST_CASE(reass_weak_from_strong){
    rcc::weak_ptr<TestSharedPtrObj> ptr(state);
    ptr = this->ptr;
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE

TEST_CASE(reass_weak_from_weak){
    rcc::weak_ptr<TestSharedPtrObj> ptr(state);
    ptr = this->weak;
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count + 1);
} END_TEST_CASE


// move operators
TEST_CASE(remove_strong_from_weak){
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count);

    rcc::shared_ptr<TestSharedPtrObj> ptr1(state);
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);

    ptr1 = std::move(weak);
    assert(weak.GetState() == nullptr);

    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count);
    start_state_count -= 1;
}END_TEST_CASE

TEST_CASE(remove_strong_from_strong){
    rcc::shared_ptr<TestSharedPtrObj> ptr(state);
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
    ptr = std::move(this->ptr);
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count);
    start_obj_count -= 1;
    start_state_count -= 1;
} END_TEST_CASE

TEST_CASE(remove_weak_from_strong){
    rcc::weak_ptr<TestSharedPtrObj> ptr(state);
    ptr = std::move(this->ptr);
    assert(state->ObjectCount() == start_obj_count - 1);
    assert(state->StateCount() == start_state_count);
    start_obj_count -= 1;
    start_state_count -= 1;
} END_TEST_CASE

TEST_CASE(remove_weak_from_weak){
    rcc::weak_ptr<TestSharedPtrObj> ptr(state);
    ptr = std::move(this->weak);
    assert(state->ObjectCount() == start_obj_count);
    assert(state->StateCount() == start_state_count);
    start_state_count -= 1;
} END_TEST_CASE
*/



int main(){
    {
        Fixture_casting().testCase();
        Fixture_construct().testCase();
        Fixture_typed_pointer().testCase();
        Fixture_derived_pointer().testCase();
        /*auto ctrs = PerModuleInterface::GetInstance()->GetConstructors();
        IObjectConstructor* ctr = ctrs[0];

        rcc::shared_ptr<TestSharedPtrObj> ptr(ctr->Construct());
        IObjectSharedState* state = ptr.GetState();
        assert(state->ObjectCount() == 1);
        assert(state->StateCount() == 1);

        rcc::shared_ptr<TestSharedPtrObj> ptr2(state);
        assert(state->ObjectCount() == 2);
        assert(state->StateCount() == 2);
        rcc::weak_ptr<TestSharedPtrObj> weak(state);
        assert(state->ObjectCount() == 2);
        assert(state->StateCount() == 3);

        Fixture_ctr_strong_from_weak(state).testCase();
        Fixture_ctr_strong_from_strong(state).testCase();
        Fixture_ctr_weak_from_strong(state).testCase();
        Fixture_ctr_weak_from_weak(state).testCase();
        assert(!destroyed);

        Fixture_ass_strong_from_weak(state).testCase();
        Fixture_ass_strong_from_strong(state).testCase();
        Fixture_ass_weak_from_strong(state).testCase();
        Fixture_ass_weak_from_weak(state).testCase();
        assert(!destroyed);

        Fixture_move_strong_from_weak(state).testCase();
        Fixture_move_strong_from_strong(state).testCase();
        Fixture_move_weak_from_strong(state).testCase();
        Fixture_move_weak_from_weak(state).testCase();
        assert(!destroyed);

        Fixture_reass_strong_from_weak(state).testCase();
        Fixture_reass_strong_from_strong(state).testCase();
        Fixture_reass_weak_from_strong(state).testCase();
        Fixture_reass_weak_from_weak(state).testCase();
        assert(!destroyed);

        Fixture_remove_strong_from_weak(state).testCase();
        Fixture_remove_strong_from_strong(state).testCase();
        Fixture_remove_weak_from_strong(state).testCase();
        Fixture_remove_weak_from_weak(state).testCase();
        assert(!destroyed);*/
    }
    /*assert(destroyed);
    std::cout << "All is well" << std::endl;*/
    return 0;
}

