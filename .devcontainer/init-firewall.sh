#!/bin/bash
set -euo pipefail  # Exit on error, undefined vars, and pipeline failures
IFS=$'\n\t'       # Stricter word splitting

# 1. Extract Docker DNS info BEFORE any flushing
DOCKER_DNS_RULES=$(iptables-save -t nat | grep "127\.0\.0\.11" || true)

# Flush existing rules and delete existing ipsets
iptables -F
iptables -X
iptables -t nat -F
iptables -t nat -X
iptables -t mangle -F
iptables -t mangle -X
ipset destroy allowed-domains 2>/dev/null || true

# 2. Selectively restore ONLY internal Docker DNS resolution
if [ -n "$DOCKER_DNS_RULES" ]; then
    echo "Restoring Docker DNS rules..."
    iptables -t nat -N DOCKER_OUTPUT 2>/dev/null || true
    iptables -t nat -N DOCKER_POSTROUTING 2>/dev/null || true
    echo "$DOCKER_DNS_RULES" | xargs -L 1 iptables -t nat
else
    echo "No Docker DNS rules to restore"
fi

# First allow DNS and localhost before any restrictions
# Allow outbound DNS
iptables -A OUTPUT -p udp --dport 53 -j ACCEPT
# Allow inbound DNS responses
iptables -A INPUT -p udp --sport 53 -j ACCEPT
# Allow outbound SSH
iptables -A OUTPUT -p tcp --dport 22 -j ACCEPT
# Allow inbound SSH responses
iptables -A INPUT -p tcp --sport 22 -m state --state ESTABLISHED -j ACCEPT
# Allow localhost
iptables -A INPUT -i lo -j ACCEPT
iptables -A OUTPUT -o lo -j ACCEPT

# Create ipset for individual IPs (dnsmasq adds IPs, not CIDRs)
ipset create allowed-domains hash:ip

# Allowed domains — dnsmasq matches these as suffixes, so e.g. "github.com"
# covers api.github.com, *.github.com, etc.
ALLOWED_DOMAINS=(
    github.com              # api.github.com, *.github.com
    githubusercontent.com   # raw content, avatars, user content
    blob.core.windows.net   # productionresultssa{1-19}, vscode, etc.
    npmjs.org               # registry.npmjs.org
    anthropic.com           # api.anthropic.com, statsig.anthropic.com
    sentry.io
    statsig.com
    visualstudio.com        # marketplace.visualstudio.com
    code.visualstudio.com   # update.code.visualstudio.com
)

# Build the dnsmasq --ipset directive: /domain1/domain2/.../ipset_name
IPSET_DIRECTIVE="/"
for domain in "${ALLOWED_DOMAINS[@]}"; do
    IPSET_DIRECTIVE+="${domain}/"
done
IPSET_DIRECTIVE+="allowed-domains"

# Capture the upstream DNS server before we overwrite resolv.conf.
# On the default bridge network this is the host/VM DNS (e.g. 192.168.65.7),
# not Docker's embedded DNS at 127.0.0.11 (which only exists in user-defined networks).
UPSTREAM_DNS=$(awk '/^nameserver/ {print $2; exit}' /etc/resolv.conf)
if [ -z "$UPSTREAM_DNS" ]; then
    echo "ERROR: No nameserver found in /etc/resolv.conf"
    exit 1
fi
echo "Upstream DNS: $UPSTREAM_DNS"

# Set up dnsmasq as a forwarding DNS resolver that populates the ipset.
# For listed domains, dnsmasq automatically adds resolved IPs to the ipset,
# handling IP rotation without needing the GitHub meta API.
echo "Starting dnsmasq..."
dnsmasq \
    --no-resolv \
    --server="$UPSTREAM_DNS" \
    --listen-address=127.0.0.1 \
    --bind-interfaces \
    --user=root \
    --ipset="$IPSET_DIRECTIVE"

# Verify dnsmasq is actually running (it daemonizes, so a successful exit code
# from the parent doesn't guarantee the child is alive).
if ! pgrep -x dnsmasq >/dev/null; then
    echo "ERROR: dnsmasq failed to start"
    exit 1
fi

# Point DNS resolution at dnsmasq
echo "nameserver 127.0.0.1" > /etc/resolv.conf

# Verify dnsmasq is working by doing a test resolution
echo "Verifying dnsmasq is resolving..."
if ! dig +short +timeout=5 @127.0.0.1 api.github.com >/dev/null 2>&1; then
    echo "ERROR: dnsmasq is not resolving queries"
    exit 1
fi
echo "dnsmasq is working"

# Get host IP from default route
HOST_IP=$(ip route | grep default | cut -d" " -f3)
if [ -z "$HOST_IP" ]; then
    echo "ERROR: Failed to detect host IP"
    exit 1
fi

HOST_NETWORK=$(echo "$HOST_IP" | sed "s/\.[0-9]*$/.0\/24/")
echo "Host network detected as: $HOST_NETWORK"

# Set up remaining iptables rules
iptables -A INPUT -s "$HOST_NETWORK" -j ACCEPT
iptables -A OUTPUT -d "$HOST_NETWORK" -j ACCEPT

# Set default policies to DROP first
iptables -P INPUT DROP
iptables -P FORWARD DROP
iptables -P OUTPUT DROP

# First allow established connections for already approved traffic
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

# Then allow only specific outbound traffic to allowed domains
iptables -A OUTPUT -m set --match-set allowed-domains dst -j ACCEPT

# Explicitly REJECT all other outbound traffic for immediate feedback
iptables -A OUTPUT -j REJECT --reject-with icmp-admin-prohibited

echo "Firewall configuration complete"
echo "Verifying firewall rules..."
if curl --connect-timeout 5 https://example.com >/dev/null 2>&1; then
    echo "ERROR: Firewall verification failed - was able to reach https://example.com"
    exit 1
else
    echo "Firewall verification passed - unable to reach https://example.com as expected"
fi

# Verify GitHub API access
if ! curl --connect-timeout 5 https://api.github.com/zen >/dev/null 2>&1; then
    echo "ERROR: Firewall verification failed - unable to reach https://api.github.com"
    exit 1
else
    echo "Firewall verification passed - able to reach https://api.github.com as expected"
fi
