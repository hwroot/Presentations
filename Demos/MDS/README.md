# How to run PoC

1) Install correct VC redistributive or recompile PoCs using Visual Studio
2) Run l1tf_victim first
3) Then run l1tf_attacker
4) Wait for leaked buffer (64 bytes total) to appear in attacker window 
5) You can close and re-run attacker process as many times as you want
