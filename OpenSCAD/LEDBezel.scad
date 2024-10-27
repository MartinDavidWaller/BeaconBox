 //CDDiameterMM = 121;
 //Height = 20;
 //LipRadiusMM = 5;
 
 centralCylinderHeight = 20;
 centralCylinderRadius = 3.5;
 
 outerCylinderHeight = 3;
 outerCylinderRadius = 5;
 
 coneCylinderHeight = 2;
 coneOuterCylinderRadius = 6; 
 
 $fa = 1; $fs = $preview ? 2 : 0.5;
 
 difference() {
 union() {
   cylinder(h=outerCylinderHeight, r=outerCylinderRadius , center=false);
     translate([0,0,outerCylinderHeight])
   cylinder(h=coneCylinderHeight, r1=coneOuterCylinderRadius, r2=0, center=false);
   
 }
 translate([0,0,-1])
 cylinder(h=centralCylinderHeight, r=centralCylinderRadius , center=false);
 }
