#include <RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <IObject.h>
#include <iostream>

bool destroyed = false;

class TestSharedPtrObj: public IObject{
public:
    ~TestSharedPtrObj(){
        destroyed = true;
    }
};

struct Fixture{
    Fixture(IObjectSharedState* state_, const char* name_)
        : state(state_)
        , weak(state_)
        , ptr(state_)
        , name(name_){
        start_state_count = state_->StateCount();
        start_obj_count = state_->ObjectCount();
    }
    ~Fixture(){
        assert(state);
        assert(!destroyed);
        assert(state->StateCount() == start_state_count);
        assert(state->ObjectCount() == start_obj_count);
    }

    IObjectSharedState* state;
    int start_state_count = 0;
    int start_obj_count = 0;
    const char* name = nullptr;
    rcc::shared_ptr<TestSharedPtrObj> ptr;
    rcc::weak_ptr<TestSharedPtrObj> weak;
};

#define TEST_CASE(NAME) struct Fixture_##NAME: public Fixture{ \
    Fixture_##NAME(IObjectSharedState* state_):\
      Fixture(state_, #NAME){} \
    void testCase()

#define END_TEST_CASE  };

TEST_CASE(ctr_strong_from_weak){
    rcc::shared_ptr<TestSharedPtrObj> ptr(weak);
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
}END_TEST_CASE

TEST_CASE(ctr_strong_from_state){
    rcc::shared_ptr<TestSharedPtrObj> ptr(state);
    assert(state->ObjectCount() == start_obj_count + 1);
    assert(state->StateCount() == start_state_count + 1);
}END_TEST_CASE

TEST_CASE(ctr_strong_from_strong){
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


REGISTERCLASS(TestSharedPtrObj);

int main(){
    {
        auto ctrs = PerModuleInterface::GetInstance()->GetConstructors();
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
        assert(!destroyed);
    }
    assert(destroyed);
    std::cout << "All is well" << std::endl;
    return 0;
}
