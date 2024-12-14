package com.adrians.groupchatturing

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChatBubble
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable
fun LobbyScreen(viewModel: MainViewModel)
{
    val lobbyId by viewModel.lobbyId.collectAsState()
    val userName by viewModel.userName.collectAsState()
    val isLobbyOwner by viewModel.isHost.collectAsState()
    val userList by viewModel.lobbyUserList.collectAsState()
    val roomData by viewModel.roomData.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Text(
            text = "Lobby owned by: $userName",
            fontSize = 20.sp,
            textAlign = TextAlign.Center,
            fontWeight = FontWeight.SemiBold,
            modifier = Modifier.align(Alignment.CenterHorizontally)
        )
        Text(
            text = "Join code: $lobbyId",
            fontSize = 24.sp,
            textAlign = TextAlign.Center,
            fontWeight = FontWeight.SemiBold,
            modifier = Modifier
                .align(Alignment.CenterHorizontally)
                .padding(6.dp)
        )
        Text(
            text = "${userList.size} out of ${roomData["maxUsers"]} users.",
            fontSize = 24.sp,
            textAlign = TextAlign.Center,
            fontWeight = FontWeight.SemiBold,
            modifier = Modifier.align(Alignment.CenterHorizontally)
        )
        LazyColumn(
            modifier = Modifier
                .fillMaxWidth()
                .weight(1f), // Let it take available vertical space
            verticalArrangement = Arrangement.spacedBy(8.dp),
            horizontalAlignment = Alignment.Start
        ) {
            items(userList) { user ->
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(vertical = 4.dp),
                    border = BorderStroke(1.dp, Color.Gray), // Add a visible border
                    shape = RoundedCornerShape(8.dp), // Optional: Rounded corners for better visuals
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(16.dp) // Padding inside each cell
                    ) {
                        Text(
                            text = user,
                            fontSize = 22.sp,
                            color = Color.White,
                            modifier = Modifier.align(Alignment.CenterStart)
                        )
                    }
                }
//                Row(
//                    modifier = Modifier
//                        .fillMaxWidth()
//                        .padding(horizontal = 8.dp),
//                    verticalAlignment = Alignment.CenterVertically,
//                    horizontalArrangement = Arrangement.SpaceBetween
//                ) {
//                    Text(text = user, fontSize = 22.sp, color = Color.LightGray)
//                }
            }
        }
        if (!isLobbyOwner)
        {
            Text(
                text = "Wait for the owner to start the game",
                fontSize = 14.sp,
                textAlign = TextAlign.Center,
                fontWeight = FontWeight.Thin,
                modifier = Modifier.align(Alignment.CenterHorizontally)
            )
        }
//        userList.forEach { user ->
//            Row(
//                modifier = Modifier.fillMaxWidth(),
//                verticalAlignment = Alignment.CenterVertically,
//                horizontalArrangement = Arrangement.SpaceBetween
//            ) {
//                Text(text = user, fontSize = 22.sp, color = Color.Green)
//            }
//        }
//        Row(
//            horizontalArrangement = Arrangement.spacedBy(34.dp) // Space between buttons
//        )
//        {
            Button(onClick = { viewModel.startGame() }, enabled = isLobbyOwner) {
                Text(text = "Start Game ")
                Icon(imageVector = Icons.Default.ChatBubble, contentDescription = null)
            }
//        }
    }
}
