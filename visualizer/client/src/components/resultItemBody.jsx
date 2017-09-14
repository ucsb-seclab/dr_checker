
import React from 'react';
import PropTypes from 'prop-types';
import { CardContent } from 'material-ui/Card';
import Collapse from 'material-ui/transitions/Collapse';
import Divider from 'material-ui/Divider';
import Typography from 'material-ui/Typography';

/**
 * This class represent the body of the resultItem.
 * It encapsulatesvarious subcomponents responsble to show for example
 * the highlighted source code, the summary of the analysis relative to this
 * this function, etc...
 */
function ResultItemHeader(props) {
  return (
    <Collapse in={props.expanded} transitionDuration="auto" unmountOnExit>
      <Divider />
      <CardContent>
        <Typography paragraph type="body2">
                Method:
        </Typography>
        <Typography paragraph>
                Heat 1/2 cup of the broth in a pot until simmering, add saffron and set aside for 10
                minutes.
        </Typography>
        <Typography paragraph>
                Heat oil in a (14- to 16-inch) paella pan or a large, deep skillet over medium-high
                heat. Add chicken, shrimp and chorizo, and cook, stirring occasionally until lightly
                browned, 6 to 8 minutes. Transfer shrimp to a large plate and set aside, leaving
                chicken and chorizo in the pan. Add pimentón, bay leaves, garlic, tomatoes, onion,
                salt and pepper, and cook, stirring often until thickened and fragrant, about 10
                minutes. Add saffron broth and remaining 4 1/2 cups chicken broth; bring to a boil.
        </Typography>
        <Typography paragraph>
                Add rice and stir very gently to distribute. Top with artichokes and peppers, and
                cook without stirring, until most of the liquid is absorbed, 15 to 18 minutes.
                Reduce heat to medium-low, add reserved shrimp and mussels, tucking them down into
                the rice, and cook again without stirring, until mussels have opened and rice is
                just tender, 5 to 7 minutes more. (Discard any mussels that don’t open.)
        </Typography>
        <Typography>
                Set aside off of the heat to let rest for 10 minutes, and then serve.
        </Typography>
      </CardContent>
    </Collapse>
  );
}

ResultItemHeader.propTypes = {
  expanded: PropTypes.bool.isRequired,
};

export default ResultItemHeader;
