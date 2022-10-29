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

