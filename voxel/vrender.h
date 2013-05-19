/*  
	Copyright (c) 2013, Alexey Saenko
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/ 

#ifndef VRENDER_H
#define VRENDER_H

typedef unsigned char uint8;

namespace Render {
	void	initRender(int scrWidth, int scrHeight);
	void	destroyRender();

	void	render();
	void	move(float dx, float dy, float dz); 
	void	turn(float da); 

	uint8	*getFrameBuffer();
	int		getFrameBufferSize();
	int		getWidth();
	int		getHeight();

	void	initData(int i);
};



#endif
