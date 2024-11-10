//eslint-disable no-undef
let width = 500;
let height = 500;
let GM = 50;
const FOLLOWING_DISTANCE = 50;
const RADIUS_GROWTH_RATE = 0.02;
const STARTING_SIZE = 3;

class Ball {
	constructor(x, y, radius) {
		this.x = x;
		this.y = y;
		this.radius = radius;
		this.orbitRadius = random(width);
		this.offSet = random(Number.MAX_SAFE_INTEGER);
		this.color = color(random(100, 235), random(100, 235), random(100, 235));
		this.dead = false;
		//Internally fix the width and height at time of creation to avoid strange animation effects.
		this.fixedW = width;
		this.fixedH = height;
	}

	Update() {
		if (this.x > width + this.radius || this.y > height) {
			//Resets the "balls" starting position once it moves off screen;
			this.x = width / 2;
			this.y = height / 2;
			this.orbitRadius = STARTING_SIZE * random(4, 8);
			this.offSet = random(Number.MAX_SAFE_INTEGER);
			this.radius = STARTING_SIZE;
			this.fixedW = width;
			this.fixedH = height;
			this.dead = true;
		}

		// The following two assignments are part of an orbital equation I found online.
		this.y =
			this.orbitRadius *
					sin(this.offSet / GM)	
					 +
			this.fixedH / 2;
		this.x =
			this.orbitRadius *
				tan(this.offSet / GM) + 
			this.fixedW / 2;

		// Adds a nice contained tracking effect to the Orbit. Note that it's constrained.
		this.x += map(
			mouseX - width / 2,
			-width / 2,
			width / 2,
			(-1 * width) / FOLLOWING_DISTANCE,
			width / FOLLOWING_DISTANCE
		);
		this.y += map(
			mouseY - height / 2,
			-height / 2,
			height / 2,
			(-1 * height) / FOLLOWING_DISTANCE,
			height / FOLLOWING_DISTANCE
		);

		// All numbers are arbitrary, offSet is responsible for increasing the input to the orbital function.
		this.orbitRadius += 1; 
		this.radius += 0 //RADIUS_GROWTH_RATE;
		this.offSet += 1 //*  cos(this.radius);
	}
	DrawBall() {
		fill(this.color);
		// eslint-disable-next-line no-undef
		noStroke();
		// eslint-disable-next-line no-undef
		ellipse(this.x, this.y, this.radius, this.radius);
	}
}

let balls = [];
let lerpBackground = 225;
let fRate = 1/60;


function windowResized() {
	// Resizes canvas and makes changes in the computation when size of screen changes.
	resizeCanvas(windowWidth, windowHeight);
}
function setup() {
	width = windowWidth;
	height = windowHeight;
	createCanvas(windowWidth, windowHeight);
	
	frameRate(60);
	for (let i = 0; i < width / 2; i++) {
		balls.push(new Ball(width / 2, height / 2, STARTING_SIZE));
	}

	// For whatever reason there is a ring of balls upon every startup despite the starting point being random.
	// Do not understand why this is and this is my really dirty hack to get around it. Fix this in the future!

	// for (let i = 0; i < 100; i++) {
	// 	for (ball of balls) {
	// 		ball.Update();
	// 	}
	// }
}

function draw() {

	background(lerpBackground);
	for (ball of balls) {
		if(!ball.dead){
		ball.DrawBall();
		ball.Update();
		
}
	}
	
	// Makes a smooth transition from a white screen to greyscale(52)s
	lerpBackground = lerpBackground > 52 ? lerpBackground + -2 : lerpBackground;
}