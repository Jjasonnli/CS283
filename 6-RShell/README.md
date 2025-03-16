1.
The client looks for a special end marker: the EOF character (0x04).
And since data may arrive in chunks, the client keeps calling recv() until it sees EOF.

2.
We can add end markers, responses end with EOF, commands end with (0\)
If we don't mark messages data might be cut off.

3.
Stateful : remembers previous interactions
Stateless : Each request is independent of itself, no remembering previous interactions

4.
UDP although unreliable is fast, has low delay, and is used in cases where speed matters more.

5. 
The Socket API