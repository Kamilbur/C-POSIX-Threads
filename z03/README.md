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

