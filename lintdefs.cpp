//lint -esym(40, errno)

//lint -e10   Expecting ';'
//lint -esym(18, sprintf, FILETIME)  Symbol redeclared (ellipsis) conflicts with ...
//lint -e534  Ignoring return value of function
//lint -e713  Loss of precision (arg. no. 2) (unsigned int to long)
//lint -e737  Loss of sign in promotion from int to unsigned long
//lint -e754  local struct member not referenced

//  These two warnings mean that lint wants extern "C" added to file,
//  because it is a .h file, not a .hpp file.
//  This is not an issue that the compilers actually care about.
//lint -e1065  Symbol not declared as "C" conflicts with self
//lint -e1066  Symbol declared as "C" conflicts with self
//lint -e1709  typedef declared as "C" conflicts with self

//  deal with ambiguities in MediaInfoDll.cpp
//lint -esym(526, get_mi_info)  //  symbol not defined

//  unicode warnings
//lint -e559  Size of argument inconsistent with format

//lint -esym(526, _wstat, _wopen, atoi, exit)
//lint -esym(628, _wstat, _wopen, atoi, exit)
//lint -esym(746, _wstat, _wopen, atoi, exit)
//lint -esym(1055, _wstat, _wopen)

//  more common_funcs warnings
//lint -e740   Unusual pointer cast (incompatible indirect types)
//lint -e1786  Implicit conversion to Boolean (arg. no. 1) (int to bool)
//lint -e1776  Converting a string literal to char * is not const safe (arg. no. 2)

//lint -e734   Loss of precision (assignment) (31 bits to 15 bits)
//lint -e732   Loss of sign (arg. no. 3) (int to unsigned long)

//lint -esym(401, __created)   Symbol not referenced
//lint -esym(528, __created)   Symbol not referenced
//lint -esym(843, __created)   Variable could be declared as const

//  warnings/errors caused by PcLint not supporting STL, or C++11 standard
//lint -e2     Unclosed Quote
//lint -e18    Symbol redeclared (basic) 
//lint -e19    Useless Declaration
//lint -e26    Expected an expression, found ')'
//lint -e30    Expected an integer constant
//lint -e32    Field size (member 'std::_Vector_base<int,int>::_Vector_impl::_Tp_alloc_type')
//lint -e36    redefining the storage class of symbol 
//lint -e38    Offset of symbol 'std::_Vector_base<int,int>::_Vector_impl_data::pointer'
//lint -e40    Undeclared identifier 'make_unique'
//lint -e46    field type should be an integral or enumeration type
//lint -e48    Bad type
//lint -e52    Expected an lvalue
//lint -e53    Expected a scalar
//lint -e55    Bad type
//lint -e58    Bad type
//lint -e61    Bad type
//lint -e63    Expected an lvalue
//lint -e64    Type mismatch (initialization) (struct ffdata * = int)
//lint -e78    Symbol ... typedef'ed at line ... used in expression
//lint -e110   Attempt to assign to void
//lint -e151   Token 'flist' inconsistent with abstract type
//lint -e155   Ignoring { }'ed sequence within an expression, 0 assumed
//lint -e503   Boolean argument to relational
//lint -e515   Symbol has arg. count conflict 
//lint -e516   Symbol has arg. type conflict 
//lint -e521   Highest operation, a 'constant', lacks side-effects
//lint -e522   Highest operation, operator '!=', lacks side-effects
//lint -e526   Symbol not defined
//lint -e530   Symbol not initialized
//lint -e550   Symbol not accessed
//lint -e559   Size of argument no. 2 inconsistent with format
//lint -e628   no argument information provided for function
//lint -e681   Loop is not entered
//lint -e746   call to function not made in the presence of a prototype
//lint -e808   No explicit type given symbol 'file', int assumed
//lint -e1013  Symbol not a member of class ''
//lint -e1015  Symbol '_M_allocate' not found in class
//lint -e1039  Symbol is not a member of
//lint -e1040  Symbol is not a legal
//lint -e1054  template variable declaration expects a type, int assumed
//lint -e1055  Symbol undeclared, assumed to return int
//lint -e1057  Member '__gnu_cxx::__is_integer_nonstrict<<1>>::__value' cannot be used
//lint -e1062  template must be either a class or a function
//lint -e1070  No scope in which to find symbol 'pointer'
//lint -e1072  Reference variable 'file' must be initialized
//lint -e1077  Could not evaluate default template parameter '=typename _Alloc::value_type'
//lint -e1087  Previous declaration of '__gnu_cxx::__is_signed' (line 88) is incompatible
//lint -e1098  Function template specialization does not match any function template
//lint -e1514  Creating temporary to copy 'int' to 'struct ffdata &' (context: arg. no. 1)
//lint -e1712  default constructor not defined for class 'ffdata'
//lint -e1025  No function matches invocation 
//lint -e1066  Symbol declared as "C" conflicts ...
//lint -e1776  Converting a string literal is not const safe (initialization)

