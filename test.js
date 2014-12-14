function log(e, d)
{
  console.log('log: ' + e + ', ' + d);
}

log(null, 'step 0');
var extrudo = require('./index.js');

log(null, 'step 1');
var x = new extrudo.Slicer();

x.slice('3D_Knot.stl', log);

var xcount = 2;
if(true)
{
  setInterval(function (){
    console.log('<<<< loop ' + xcount);
     
    if(xcount <= 0)
    {
       process.exit(0);
    }
    xcount -= 1;
  },1000);
}
