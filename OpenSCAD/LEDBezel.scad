
 
 centralCylinderHeight = 20;
 centralCylinderRadius = 3.75;
 
 outerCylinderHeight = 4;
 outerCylinderRadius = 6;
 
 coneCylinderHeight = 5;
 coneOuterCylinderRadius = 8; 
 
 $fa = 1; $fs = $preview ? 2 : 0.5;
 
 difference() {
 union() {
   cylinder(h=outerCylinderHeight, r=outerCylinderRadius , center=false);
     translate([0,0,outerCylinderHeight])
   cylinder(h=coneCylinderHeight, r1=coneOuterCylinderRadius, r2=0, center=false);

   
 }
    translate([0,0,-1])
        cylinder(h=centralCylinderHeight, r=centralCylinderRadius , center=false);
 
    // Square hole prints badly
 
    //translate([-2.55,-2.55,-1])
    //cube([5.5,5.5,30],false);
 
      translate([0,-2.0,-1])
     cube([10,4,3],false);
 }
