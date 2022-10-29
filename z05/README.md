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


