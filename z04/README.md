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

