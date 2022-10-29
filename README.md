# POSIX Threads exercises


### Problem 1

Write a readers-writers problem solution, that allows parallel reads by multiple readers.


<table>
<tr>
  <td>
    Listing 5.1: Solution to readers-writers problem using semaphores
  </td>
</tr>
<tr>
  <td>
   <pre>
    var readersSem, writerSem: semaphore;
        readersCount: integer;
 
    // Writer thread:
    wait (writerSem);
    ...
    // writing
    ...
    release (writerSem);


    // Reader thread:
    wait (readersSem);
    readersCount++;
    if (readersCount == 1)
        wait (writerSem);
    release (readersSem);
    ...
    // reading
    ...
    wait (readersSem);
    readersCount--;
    if (readersCount == 0)
        release (writerSem);
    release (readersSem);
   </pre>
  </td>
</tr>
</table>


### Problem 2

Assume that there is a buffer (of some constant size) and the writers can write to the buffer. Thanks to this, even if readers are reading, a writer does not have to wait for resources, but writes to the buffer (if buffer is not full). The buffer is freed by the readers.

<table>
 <tr>
  <td>
   <b>
   Note:
   </b>
   The buffer may be represented by a special writer, which is written to by the other writers. 
  </td>
 </tr>
</table>


<table>
 <tr>
  <td>
   Listing 5.2: Pisanie z wykorzystaniem bufora
  </td>
 </tr>
 <tr>
  <td>
   <pre>
[ Writer 1 ] \                      / [ Reader 1 ]
[ Writer 2 ] --&gt; [ Buffer ] -&gt; () &lt;-- [ Reader 2 ]
[ Writer 3 ] /                      \ [ Reader 3 ]
   </pre>
  </td>
 </tr>
</table>


### Problem 3

<p>
Assume there is a couple of objects. Writers write to them and readers read their content.
</p>
<table>
 <tr>
  <td>
  Listing 5.3: Wiele obiektów dostępnych dla czytelników/pisarzy
  </td>
 </tr>
 <tr>
  <td>
   <pre>
[ Writer 1 ] \          / [ Reader 1 ]
[ Writer 2 ]  \   ()   /  [ Reader 2 ]
[ Writer 3 ]  --&gt; () &lt;--  [ Reader 3 ]
[ Writer 4 ]  /   ()   \  [ Reader 4 ]
[ Writer 5 ] /          \ [ Reader 5 ]
   </pre>
  </td>
 </tr>
</table>

<table>
 <tr>
  <td><b>Note: </b>
  Solution may be based on the array of mutexes, all of which guard access to certain object. For access to mutexes you can use function <span class="code">pthread_mutex_trylock</span>.
  </td>
 </tr>
</table>

### Problem 4

<p>
Assume there is a "critic", who writes the data after all writers have written something at least once. Critic should be informed about this using conditions mechanism.
</p>
<table>
 <tr>
  <td>
  Listing 5.4: Critic vs. Writers
  </td>
 </tr>
 <tr>
  <td>
   <pre>
 
 [ Writer 1 ] \          / [ Reader 1 ]
 [ Writer 2 ]  \        /  [ Reader 2 ]
 [ Writer 3 ]  --&gt; () &lt;--  [ Reader 3 ]
   &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; /        \  [ Reader 4 ]
 [  Critic  ] /          \ [ Reader 5 ]
 
   </pre>
  </td>
 </tr>
</table>

### Problem 5

<p>
A variant of problem 3. Assume we have some objects and that writers write and readers read from those objects. Additionaly every object has maximal number of readers that can read from it simultaneously.  
</p>
<table>
 <tr>
  <td>
  Listing 5.5: Multiple objects available for readers/writers
  </td>
 </tr>
 <tr>
  <td>
   <pre>

[ Writer 1 ] \           / [ Reader 1 ]
[ Writer 2 ]  \   (2)   /  [ Reader 2 ]
[ Writer 3 ]  --&gt; (3) &lt;--  [ Reader 3 ]
[ Writer 4 ]  /   (2)   \  [ Reader 4 ]
[ Writer 5 ] /           \ [ Reader 5 ]
   </pre>
  </td>
 </tr>
</table>


### Problem 6

<p>
Assume that writers write only when the buffer is empty. They write until it is full and then they inform readers that the buffer is ready to read. At that time readers read it all until it is empty and writers are informed that buffer is empty.
</p>
<table>
 <tr>
  <td>
  Listing 5.6: Writing with a use of buffer
  </td>
 </tr>
 <tr>
  <td>
   <pre>

[ Writer 1 ] \                  / [ Reader 1 ]
[ Writer 2 ] --&gt; [ Bufor ] &lt;-- [ Reader 2 ]
[ Writer 3 ] /                  \ [ Reader 3 ]

   </pre>
  </td>
 </tr>
</table>

<hr>
<p>
Source: <a href="http://home.agh.edu.pl/~kozlak/PS2010/synchronizacja_linux.html"> Problems statements </a>
</p>
