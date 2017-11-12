pragma solidity ^0.4.4;

contract shapeCalulator{
     function rectangles(uint w,uint h)returns(uint s,uint p){
     s = w * h;
     p= 2* (w+h);
   }
}
