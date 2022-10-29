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

