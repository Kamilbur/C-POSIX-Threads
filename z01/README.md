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

