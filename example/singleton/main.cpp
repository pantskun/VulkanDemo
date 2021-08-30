#include <iostream>
#include <vector>

class Singleton
{
  public:
    static Singleton &Instance()
    {
        static Singleton instance_;
        return instance_;
    }

  private:
    Singleton()
    {
        printf("singleton constructor\n");
    }

  private:
    // static Singleton instance_;
};

class A
{
  public:
    A()
    {
        printf("construct A\n");
    }
    A(const A &a)
    {
        printf("copy construct A\n");
    }

    int get()
    {
        return v;
    }
    int const get() const
    {
        return v;
    }

    int v = 0;
};

int main()
{
    std::vector<A> as;
    {
        A a;
        as.push_back(a);
    }

    auto a = as.at(0);
    printf("%d\n", a.v);
    return 0;
}
