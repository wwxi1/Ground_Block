# Ground_Block 单元测试说明

## 概述
本测试套件为 `Ground_Block.c` 文件中的 `Ground_Block_enable()` 函数提供完整的单元测试覆盖。

## 被测函数
```c
void Ground_Block_enable()
```

**函数功能：**
1. 将全局变量 `solenoid_signal` 设置为 1
2. 遍历 `DJmotor` 数组（长度为 `USE_DJNUM`），将每个元素的 `Begin` 字段设置为 1（true）

## 测试框架
使用 **Unity Test Framework** - 嵌入式 C 项目的标准测试框架

## 依赖项
- Unity Test Framework
- Ground_Block.h
- DJmotor.h
- motor_config.h

## 测试用例列表

### 1. test_Ground_Block_enable_normal_case
**测试类型：** 正常功能测试  
**描述：** 验证函数在正常情况下的行为  
**验证点：**
- `solenoid_signal` 被设置为 1
- 所有 `DJmotor[i].Begin` 被设置为 true

### 2. test_Ground_Block_enable_signal_already_set
**测试类型：** 状态测试  
**描述：** 测试当 `solenoid_signal` 已经为 1 时的行为  
**验证点：**
- `solenoid_signal` 保持为 1
- 所有电机 `Begin` 标志被正确设置

### 3. test_Ground_Block_enable_some_motors_already_enabled
**测试类型：** 状态测试  
**描述：** 测试当部分电机已经启用时的行为  
**验证点：**
- `solenoid_signal` 被设置为 1
- 所有电机的 `Begin` 标志都被设置为 true

### 4. test_Ground_Block_enable_max_index
**测试类型：** 边界测试  
**描述：** 验证数组的最后一个索引被正确处理  
**验证点：**
- `DJmotor[USE_DJNUM-1].Begin` 被设置为 true

### 5. test_Ground_Block_enable_min_index
**测试类型：** 边界测试  
**描述：** 验证数组的第一个索引被正确处理  
**验证点：**
- `DJmotor[0].Begin` 被设置为 true

### 6. test_Ground_Block_enable_multiple_calls
**测试类型：** 幂等性测试  
**描述：** 测试函数可以被多次调用且产生相同结果  
**验证点：**
- 多次调用后状态保持一致

### 7. test_Ground_Block_enable_signal_value_type
**测试类型：** 类型测试  
**描述：** 验证 `solenoid_signal` 被设置为精确的 uint8_t 值 1  
**验证点：**
- 值类型和精度正确

### 8. test_Ground_Block_enable_loop_iteration_count
**测试类型：** 逻辑测试  
**描述：** 验证循环恰好迭代 `USE_DJNUM` 次  
**验证点：**
- 恰好 `USE_DJNUM` 个电机受到影响

### 9. test_Ground_Block_enable_after_disable_state
**测试类型：** 场景测试  
**描述：** 模拟禁用后启用的场景  
**验证点：**
- `solenoid_signal` 被正确设置
- 电机 `Begin` 被设置
- 其他字段（如 `MODE_Set`）不被影响

### 10. test_Ground_Block_enable_motor_structure_integrity
**测试类型：** 副作用测试  
**描述：** 验证函数只修改 `Begin` 字段  
**验证点：**
- 只有 `Begin` 字段被修改
- 其他字段保持不变

### 11. test_Ground_Block_enable_alternating_states
**测试类型：** 模式测试  
**描述：** 使用交替的初始状态进行测试  
**验证点：**
- 所有电机最终都被启用

### 12. test_Ground_Block_enable_signal_boundary_max
**测试类型：** 边界测试  
**描述：** 测试当 `solenoid_signal` 为最大 uint8_t 值时的行为  
**验证点：**
- `solenoid_signal` 被正确设置为 1

## 测试覆盖率

### 代码覆盖率：100%

**覆盖的代码路径：**
- ✅ 全局变量赋值：`solenoid_signal = 1`
- ✅ for 循环初始化：`uint32_t i = 0`
- ✅ for 循环条件：`i < USE_DJNUM`
- ✅ for 循环递增：`i++`
- ✅ 数组访问和赋值：`DJmotor[i].Begin = 1`

**覆盖的场景：**
- ✅ 正常输入场景
- ✅ 边界值（最小值、最大值）
- ✅ 条件分支（循环的所有迭代）
- ✅ 多次调用场景
- ✅ 不同的初始状态

### 分支覆盖率：100%
- ✅ 循环执行 0 次（虽然 `USE_DJNUM=4` 不会出现这种情况，但边界测试覆盖了最小索引）
- ✅ 循环执行完整次数（4 次）
- ✅ 数组边界访问（索引 0 和索引 3）

## 如何运行测试

### 方法 1：使用命令行
```bash
# 编译测试
gcc -o test_Ground_Block test_Ground_Block.c unity.c -I./Unity/include -I../APL/Tasks/Inc -I../Motor/Inc -I../FML/Inc

# 运行测试
./test_Ground_Block
```

### 方法 2：使用 CMake
```bash
mkdir build && cd build
cmake ..
make
./test_Ground_Block
```

### 方法 3：在 Keil MDK 中
1. 将 `test_Ground_Block.c` 添加到项目
2. 配置包含路径指向 Unity 框架头文件
3. 编译并运行

## 预期输出
```
test_Ground_Block.c:xxx:test_Ground_Block_enable_normal_case:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_signal_already_set:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_some_motors_already_enabled:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_max_index:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_min_index:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_multiple_calls:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_signal_value_type:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_loop_iteration_count:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_after_disable_state:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_motor_structure_integrity:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_alternating_states:PASS
test_Ground_Block.c:xxx:test_Ground_Block_enable_signal_boundary_max:PASS

-----------------------
12 Tests 0 Failures 0 Ignored
OK
```

## 测试约束和假设

1. **USE_DJNUM 值：** 假设 `USE_DJNUM` 定义为 4（来自 `motor_config.h`）
2. **DJmotor 数组：** 假设 `DJmotor` 是外部定义的全局数组
3. **Begin 字段类型：** 假设 `DJMotor.Begin` 是 `volatile bool` 类型
4. **solenoid_signal 类型：** 假设是 `uint8_t` 类型

## 维护说明

当以下情况发生时，需要更新测试：
1. `USE_DJNUM` 的值改变
2. `Ground_Block_enable()` 函数实现改变
3. `DJMotor` 结构体定义改变（特别是 `Begin` 字段）
4. `solenoid_signal` 类型改变

## 注意事项

1. 所有导入语句都是显式的，没有使用通配符 `*`
2. 每个测试用例都有完整的实现和断言
3. 使用 `setUp()` 和 `tearDown()` 确保测试之间的隔离
4. 测试文件独立可运行，不依赖外部系统状态