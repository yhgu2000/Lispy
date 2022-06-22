#pragma once

#include <forward_list>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace lispy {

class LISPY_EXPORT Object
{
public:
  static bool gFlag;
  static std::unordered_set<Object*> gRoots;
  static std::forward_list<Object*> gAll;

public:
  /**
   * @brief 执行一次垃圾回收
   */
  static void gc();

  /**
   * @brief 设计上，所有 Object 及其子类对象必须由某个 GC 创建！
   */
  template<typename T, typename... Types>
  static T* gc_make(Types&&... args)
  {
    auto t = new T(std::forward<Types>(args)...);
    gAll.push_front(t);
    return t;
  }

  /**
   * @brief GC 标记
   */
  static void gc_mark(Object* obj)
  {
    if (obj->__flag__ != gFlag)
      obj->__flag__ = gFlag, obj->__mark__();
  }

  /**
   * @brief 将某个对象设为根对象，如果 obj 不属于此 GC 管理器，则无操作。
   */
  static void gc_lift(Object* obj) { gRoots.insert(obj); }

  /**
   * @brief 取消标记根对象，如果 obj 为其它 GC 管理的对象，则无操作。
   */
  static void gc_drop(Object* obj) { gRoots.erase(obj); }

public:
  virtual ~Object() = default;

public:
  using __Went__ = std::unordered_set<Object const*>;

  /**
   * @brief 打印对象的字符串形式，要求在恰当的上下文中可以重解释得到对象
   */
  virtual void __repr__(std::ostream& out, __Went__& went) const = 0;

  /**
   * @brief 哈希方法，用于表示对象 ID
   */
  virtual std::size_t __hash__() const { return std::size_t(this); }

  /**
   * @brief 相等性判别方法（重要）
   */
  virtual bool __equal__(Object* obj) const { return this == obj; }

  /**
   * @brief 获取对象属性，如果属性不存在返回空指针
   */
  virtual Object* __attr__(Object* obj) { return nullptr; }

protected:
  /**
   * @brief 对所引用的对象进行 GC 标记
   */
  virtual void __mark__() = 0;

private:
  bool __flag__{ gFlag };
};

class LISPY_EXPORT String
  : public Object
  , public std::string
{
public:
  using std::string::string;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual std::size_t __hash__() const override;
  virtual bool __equal__(Object* obj) const override;

protected:
  virtual void __mark__() override {}
};

class LISPY_EXPORT Null : public Object
{
public:
  static String gName;
  static Null g;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual std::size_t __hash__() const override { return SIZE_MAX; }
  virtual bool __equal__(Object* obj) const override { return this == obj; }

protected:
  virtual void __mark__() override {}

private:
  Null() = default;
};

class LISPY_EXPORT False : public Object
{
public:
  static String gName;
  static False g;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual std::size_t __hash__() const override { return 0; }
  virtual bool __equal__(Object* obj) const override { return this == obj; }

protected:
  virtual void __mark__() override {}

private:
  False() = default;
};

class LISPY_EXPORT True : public Object
{
public:
  static String gName;
  static True g;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual std::size_t __hash__() const override { return 1; }
  virtual bool __equal__(Object* obj) const override { return this == obj; }

protected:
  virtual void __mark__() override {}

private:
  True() = default;
};

class LISPY_EXPORT Number : public Object
{
public:
  double _;

public:
  Number(double v = 0)
    : _(v)
  {}

public:
  operator double() const { return _; }
  Number& operator=(double v) { return _ = v, *this; }
  double& operator*() { return _; }

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual std::size_t __hash__() const override;
  virtual bool __equal__(Object* obj) const override;

protected:
  virtual void __mark__() override {}
};

class LISPY_EXPORT Vector
  : public Object
  , public std::vector<Object*>
{
public:
  using std::vector<Object*>::vector;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;

protected:
  virtual void __mark__() override;
};

struct LISPY_EXPORT __Hash__
{
  std::size_t operator()(Object* obj) const { return obj->__hash__(); }
};

struct LISPY_EXPORT __Equal__
{
  bool operator()(Object* lhs, Object* rhs) const
  {
    return lhs->__equal__(rhs);
  }
};

using __HashMap__ = std::unordered_map<Object*, Object*, __Hash__, __Equal__>;

class LISPY_EXPORT HashMap
  : public Object
  , public __HashMap__
{
public:
  using __HashMap__::__HashMap__;

public:
  /**
   * @brief 获取键映射的值，若未找到，返回空指针。
   */
  Object* get(Object* key) noexcept
  {
    auto iter = find(key);
    if (iter == end())
      return nullptr;
    return iter->second;
  }

  /**
   * @brief 设置键值映射，指针都不允许为空。
   */
  void set(Object* key, Object* obj) noexcept { (*this)[key] = obj; }

  /**
   * @brief 删除键，指针不允许为空，若键存在，返回 true。
   */
  bool del(Object* key) noexcept { return erase(key) == 1; }

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;

public:
  virtual void __mark__() override;
};

class LISPY_EXPORT Program : public Object
{
public:
  virtual Object* exec(HashMap* ctx) = 0;

public:
  virtual void __mark__() override{};
};

class LISPY_EXPORT Macro : public Object
{
public:
  /**
   * @brief 解释程序执行，获取返回值（Lisp 的 eval 函数），返回对象受 GC
   * 系统管理，且没有被提升，可能会在下一次 GC 中被回收，使用时务必当心指针悬
   * 挂。
   */
  static Object* evaluate(HashMap* ctx, Object* obj)
  {
    auto program = dynamic_cast<Program*>(obj);
    return program == nullptr ? obj : program->exec(ctx);
  }

public:
  virtual Object* eval(HashMap* ctx,
                       Vector::iterator begin,
                       Vector::iterator const& end) = 0;

public:
  virtual void __mark__() override{};
};

/**
 * @brief 函数宏
 */
class LISPY_EXPORT Funco : public Macro
{
public:
  virtual Object* call(Vector* args) = 0;

public:
  virtual Object* eval(HashMap* ctx,
                       Vector::iterator begin,
                       Vector::iterator const& end) override;
};

}

inline std::ostream&
operator<<(std::ostream& out, lispy::Object* obj)
{
  lispy::Object::__Went__ went;
  went.insert(obj);
  obj->__repr__(out, went);
  return out;
}
