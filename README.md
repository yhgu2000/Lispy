# 一个可拓展的、极简的类Lisp解释系统（语言）

代码没有太多注释，因为感觉其都相当直白，有自描述性，可能只是编码模式的理解需要一些时间。

## 输入程序的执行过程

仅供参考：

```mermaid
flowchart LR

code("字符串形式代码\nstd::string")
--compile--> ast("抽象语法树\nboost.spirit")
--compile_ast--> prog{{"程序对象\nProgram"}}
--"exec(ctx)"--> retobj{{"返回对象"}}
```

图中六边形表示GC世界中的对象（即用户侧可见的）。

## 相关文档

- [语法语义说明](sdt.md)

- [简介PPT](introduce.pptx)

