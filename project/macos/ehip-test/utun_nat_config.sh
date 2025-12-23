#!/bin/bash
# nat_temporary.sh
# 临时 NAT 配置脚本

set -e

# 配置变量
INT_IF="utun10"      # 内部接口
EXT_IF="en1"         # 外部接口
INT_IP="10.10.0.1"   # 内部IP
INT_NET="10.10.0.0/24" # 内部网络
GATEWAY_IP="10.10.0.1" # 网关IP

echo "=== 临时 NAT 网关配置 ==="
echo "内部接口: $INT_IF"
echo "外部接口: $EXT_IF"
echo "网关IP: $GATEWAY_IP"
echo "内部网络: $INT_NET"
echo ""

# 1. 启用 IP 转发
echo "1. 启用 IP 转发..."
sudo sysctl -w net.inet.ip.forwarding=1
echo "   net.inet.ip.forwarding = $(sysctl -n net.inet.ip.forwarding)"

# 2. 创建临时 PF 规则文件
echo -e "\n2. 创建 PF 规则..."
cat > /tmp/pf_temp.rules << EOF
# 临时 NAT 规则
ext_if = "$EXT_IF"
int_if = "$INT_IF"
int_net = "$INT_NET"

# NAT 规则
nat on \$ext_if from \$int_net to any -> (\$ext_if:0)

# 允许内部到外部的流量
pass in on \$int_if from \$int_net to any
pass out on \$ext_if from any to any

# 允许已建立的连接返回
pass in on \$ext_if from any to any keep state

# 允许回环
set skip on lo0
EOF

echo "   PF 规则已保存到 /tmp/pf_temp.rules"

# 3. 应用 PF 规则
echo -e "\n3. 应用 PF 规则..."
sudo pfctl -e
sudo pfctl -f /tmp/pf_temp.rules

# 4. 显示配置状态
echo -e "\n4. 配置状态:"
echo "   PF 状态:"
sudo pfctl -s info 2>/dev/null | grep -E "Status|Enabled" || true
echo -e "\n   NAT 规则:"
sudo pfctl -s nat
echo -e "\n   IPv4 转发状态: $(sysctl -n net.inet.ip.forwarding)"

# 5. 测试命令
echo -e "\n5. 测试命令:"
echo "   在客户端机器上执行以下命令:"
echo "   sudo ifconfig <客户端接口> 10.10.0.100 netmask 255.255.255.0"
echo "   sudo route add default 10.10.0.1"
echo "   sudo route delete default  # 清理默认路由"
echo "   或使用静态IP配置: IP: 10.10.0.x, 掩码: 255.255.255.0, 网关: 10.10.0.1, DNS: 8.8.8.8"

echo -e "\n✅ 临时 NAT 配置完成!"
echo "   注意: 重启后配置会丢失"
echo "   要清理配置，运行: sudo $0 cleanup"