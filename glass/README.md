# Intro
The Glass should constantly broadcasting IR signals, and read whatever returns on XBee channel. Then pick the largest one for the user. If the client proposes changes to the Glass, a consistency has to be resolved to determine which one to show to user.

Heuristic is that:
1, We choose the largest intensity reading.
2, We prefer the one with a larger increase in intensity.