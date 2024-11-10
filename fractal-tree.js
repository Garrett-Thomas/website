
const LIMB_MULT = 9 / 10;
const ANGLE_INC = 3.14 / 7;
const DEPTH = 10;

class Limb {
	constructor(x, y, length, angle, value) {
		this.length = length;
		this.left = null;
		this.right = null;
		this.angle = angle;
		this.value = value;
		this.x1 = x;
		this.y1 = y;

		// Using the angle along with the length and starting point, we calculate the end point of the line.
		this.x2 = this.length * Math.cos(this.angle) + this.x1;
		this.y2 = this.length * Math.sin(this.angle) + this.y1;
	}

	DrawLimb() {
		let paintingColor = map(this.value, 0, DEPTH, 50, 255);
		stroke(paintingColor, paintingColor, 0);
		line(this.x1, this.y1, this.x2, this.y2);
	}
}
function setup() {
	angleMode(RADIANS);
	createCanvas(400, 400);
	background(52);
	BuildTree(new Limb(width / 2, height, 40, (3 * PI) / 2, 9), DEPTH);
}

function draw() {}

function sleep(milliseconds) {
    const date = Date.now();
    let currentDate = null;
    do {
      currentDate = Date.now();
    } while (currentDate - date < milliseconds);
  }
  
function BuildTree(node, depth) {

    // Base case, do not want to recurse when depth is 0.
	if (depth == 0) {
		return null;
	}


    // Create two new nodes, one to either side of the argument nodes then pass those in
    // to seperate BuildTree functions.
	node.left = new Limb(
		node.x2,
		node.y2,
		node.length * LIMB_MULT,
		node.angle - ANGLE_INC,
		depth
	);

	node.right = new Limb(
		node.x2,
		node.y2,
		node.length * LIMB_MULT,
		node.angle + ANGLE_INC,
		depth
	);

	BuildTree(node.left, depth - 1);
	BuildTree(node.right, depth - 1);

    // Once BuildTree is finished, then the node is drawn.
    sleep(250);
	node.DrawLimb();

    // Returns the full tree.
	return node;
}
