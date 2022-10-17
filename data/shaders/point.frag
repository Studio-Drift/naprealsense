// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 150 core

in vec4  pass_Color;

out vec4 out_Color;

void main(void)
{
	// Set output color
	out_Color = pass_Color;
}