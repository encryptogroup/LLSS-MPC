ip netns exec neon_ns0 ./build/DelayedresharingProtocol ../OptimizedCircuits/$1 2 0 172.16.1.11 172.16.1.13 $2 10 &
ip netns exec neon_ns1 ./build/DelayedresharingProtocol ../OptimizedCircuits/$1 2 1 172.16.1.12 172.16.1.11 $2 10 &
ip netns exec neon_ns2 ./build/DelayedresharingProtocol ../OptimizedCircuits/$1 2 2 172.16.1.13 172.16.1.12 $2 10
